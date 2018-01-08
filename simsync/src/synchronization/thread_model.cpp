#include "simsync/synchronization/thread_model.hpp"

#include <algorithm>

namespace simsync {

void thread_model::add_barrier(uint64_t barrier_address, size_t count)
{
  m_barriers.emplace(barrier_address, count);
}

void thread_model::approximate_broadcast(int32_t thread_id, uint64_t condition_address)
{
  m_condition_info[condition_address].broadcasters.insert(thread_id);
}

void thread_model::approximate_signal(int32_t thread_id, uint64_t condition_address)
{
  m_condition_info[condition_address].signalers.insert(thread_id);
}

void thread_model::approximate_wait(int32_t thread_id, uint64_t condition_address)
{
  m_condition_info[condition_address].waiters.insert(thread_id);
}

void thread_model::classify_condition_variables()
{
  for(auto &cv : m_condition_info) {
    if((cv.second.broadcasters.size() > 1 || cv.second.signalers.size() > 1) ||
        (cv.second.broadcasters.size() == 1 && cv.second.signalers.size() == 1)) {
      if(cv.second.waiters.size() > 1) {
        cv.second.type = condition_type::mpmc;
      } else if(cv.second.waiters.size() == 1) {
        cv.second.type = condition_type::mpsc;
      }
    } else if((cv.second.broadcasters.size() == 1 && cv.second.signalers.empty()) ||
              cv.second.broadcasters.empty() && cv.second.signalers.size() > 1) {
      if(cv.second.waiters.size() > 1) {
        cv.second.type = condition_type::spmc;
      } else if(cv.second.waiters.size() == 1) {
        cv.second.type = condition_type::spsc;
      }
    }
  }
}

transition thread_model::create(int32_t new_thread_id)
{
  transition t{};

  // add the new thread to the start queue
  m_start_queue.push_back(new_thread_id);

  // the new thread should now wake up
  t.to_wake.insert(new_thread_id);
  work(new_thread_id);

  return t;
}

transition thread_model::start(int32_t thread_id)
{
  transition t{};

  // remove the thread from the start queue
  auto it = std::find(std::begin(m_start_queue), std::end(m_start_queue), thread_id);
  if(it != std::end(m_start_queue)) {
    m_start_queue.erase(it);
  }

  // no transition changes are required
  return t;
}

transition thread_model::finish(int32_t thread_id)
{
  transition t{};

  // add the thread to the set of finished threads
  t.finished = thread_id;
  m_finished.insert(thread_id);

  // check if another thread was waiting for this one to finish
  auto const it = m_join_queue.find(thread_id);
  if(it != m_join_queue.end()) {
    // the other thread can now wake up
    t.to_wake.insert(it->second);
    work(it->second);

    // the join dependency is complete
    m_join_queue.erase(it);
  }

  return t;
}

transition thread_model::join(int32_t current_thread, int32_t target_thread)
{
  transition t{};

  if(m_finished.find(target_thread) == m_finished.end()) {
    // the target thread has not finished yet, so the current thread should wait
    t.to_sleep.insert(current_thread);
    wait(current_thread);

    // create the join dependency
    m_join_queue.emplace(target_thread, current_thread);
  } else {
    // the target thread has already finished, there is no need to wait
  }

  return t;
}

transition thread_model::acquire(int32_t thread_id, uint64_t lock_address)
{
  transition t{};

  if(!m_lock_queue[lock_address].empty()) {
    if(m_lock_queue[lock_address].front() == thread_id) {
      // the current thread already holds the lock - this can happen due to trylocks
      return t;
    }

    // someone already holds the lock, so the current thread must wait
    t.to_sleep.insert(thread_id);
    wait(thread_id);
  } else {
    // no one holds the lock
    m_thread_locks[thread_id].push_back(lock_address);
  }

  // add the current thread to the queue.
  // if the current thread is at the front, it has acquired the lock
  m_lock_queue[lock_address].push_back(thread_id);

  return t;
}

transition thread_model::release(int32_t thread_id, uint64_t lock_address)
{
  transition t{};

  if(m_lock_queue[lock_address].front() == thread_id) {
    m_lock_queue[lock_address].pop_front();
    m_thread_locks[thread_id].pop_front();
  } else {
    throw std::runtime_error("Error: thread attempted to release lock it does not hold.");
  }

  if(!m_lock_queue[lock_address].empty()) {
    // wake up the next thread waiting for the lock
    auto const next_thread = m_lock_queue[lock_address].front();
    m_thread_locks[next_thread].push_back(lock_address);

    t.to_wake.insert(next_thread);
    work(next_thread);
  }

  return t;
}

transition thread_model::barrier_wait(int32_t thread_id, uint64_t barrier_address)
{
  transition t{};

  auto &waiters = m_barrier_queue[barrier_address];
  waiters.insert(thread_id);

  if(waiters.size() == m_barriers.at(barrier_address)) {
    // wake up all waiting threads
    for(auto const &w : waiters) {
      t.to_wake.insert(w);
      work(w);
    }

    // reset the barrier
    waiters.clear();
  } else {
    t.to_sleep.insert(thread_id);
    wait(thread_id);
  }

  return t;
}

transition thread_model::condition_broadcast(uint64_t condition_address)
{
  transition t{};

  auto &cv = m_condition_info.at(condition_address);
  if(cv.waiters.empty()) {
    return t; // ineffectual broadcast
  }

  auto &waiters = m_condition_queue[condition_address];

  if(cv.type == condition_type::mpmc || cv.type == condition_type::spmc) {
    m_production[condition_address] += cv.waiters.size() - waiters.size();
  } else if(cv.type == condition_type::mpsc || cv.type == condition_type::spsc) {
    m_production[condition_address] += 1;
  } else {
    throw std::runtime_error("Error: unknown condition variable model.");
  }

  if(!waiters.empty()) {
    auto const priority_thread = waiters.front();
    waiters.pop_front();

    auto lock_address = m_silent_reacquire.at(priority_thread).front();
    m_silent_reacquire.at(priority_thread).pop_front();
    t = acquire(priority_thread, lock_address);

    if(t.to_sleep.find(priority_thread) == t.to_sleep.end()) {
      // the lock acquire was successful, wake up the priority_thread
      t.to_wake.insert(priority_thread);
      work(priority_thread);
    }

    for(auto const &waiter : waiters) {
      lock_address = m_silent_reacquire.at(waiter).front();
      m_silent_reacquire.at(waiter).pop_front();

      m_thread_locks.at(waiter).push_back(lock_address);
      m_lock_queue.at(lock_address).push_back(waiter);
    }

    waiters.clear();
  }

  return t;
}

transition thread_model::condition_signal(uint64_t condition_address)
{
  transition t{};

  auto &cv = m_condition_info.at(condition_address);
  if(cv.waiters.empty()) {
    return t; // ineffectual signal
  }

  auto &waiters = m_condition_queue[condition_address];

  if(waiters.empty()) {
    // regardless of the condition variable type, a signal always increases production by one
    m_production[condition_address]++;
  } else {
    auto const waiter = waiters.front();

    auto const lock_address = m_silent_reacquire.at(waiter).front();
    m_silent_reacquire.at(waiter).pop_front();

    t = acquire(waiter, lock_address);
    waiters.pop_front();

    if(t.to_sleep.find(waiter) == t.to_sleep.end()) {
      // the lock acquire was successful, wake up the waiter
      t.to_wake.insert(waiter);
      work(waiter);
    }
  }

  return t;
}

transition thread_model::condition_wait(int32_t thread_id, uint64_t condition_address)
{
  transition t{};

  if(m_production[condition_address] > 0) {
    m_production[condition_address]--; // consume

    return t; // no need to wait
  }

  auto const lock_address = m_thread_locks.at(thread_id).back();

  // silently release the lock
  t = release(thread_id, lock_address);
  // save lock address for silent reacquire
  m_silent_reacquire[thread_id].push_back(lock_address);

  // wait for a signal/broadcast
  m_condition_queue[condition_address].push_back(thread_id);
  t.to_sleep.insert(thread_id);
  wait(thread_id);

  return t;
}

void thread_model::work(int32_t thread_id)
{
  m_working.insert(thread_id);

  auto waiting_it = m_waiting.find(thread_id);
  if(waiting_it != m_waiting.end()) {
    m_waiting.erase(waiting_it);
  }
}

void thread_model::wait(int32_t thread_id)
{
  m_waiting.insert(thread_id);

  auto waiting_it = m_working.find(thread_id);
  if(waiting_it != m_working.end()) {
    m_working.erase(waiting_it);
  }
}
}
