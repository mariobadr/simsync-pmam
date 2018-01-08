#ifndef SIMSYNC_SYNCHRONIZER_HPP
#define SIMSYNC_SYNCHRONIZER_HPP

#include <simsync/synchronization/transition.hpp>

#include <cstdint>
#include <deque>
#include <map>
#include <set>

namespace simsync {

/**
 * The thread model.
 *
 * Keeps track of synchronization state to determine which threads should wake up or be put to sleep.
 */
class thread_model {
public:
  /**
   * Add a new barrier.
   *
   * @param barrier_address The address the barrier is located at.
   * @param count The number of threads that must wait at this barrier before continuing.
   */
  void add_barrier(uint64_t barrier_address, size_t count);

  void approximate_broadcast(int32_t thread_id, uint64_t condition_address);

  void approximate_signal(int32_t thread_id, uint64_t condition_address);

  void approximate_wait(int32_t thread_id, uint64_t condition_address);

  void classify_condition_variables();

  /**
   * Create a thread.
   *
   * Signal that a new thread has been created and will start soon.
   *
   * @param new_thread_id The ID of the thread to be created.
   *
   * @return signal that the new_thread_id should be woken up so that it can simsync::thread_start.
   */
  transition create(int32_t new_thread_id);

  /**
   * Start a thread.
   *
   * Signal that a thread is now executing on a core.
   *
   * @param thread_id The thread that has started.
   *
   * @return no transitions.
   */
  transition start(int32_t thread_id);

  /**
   * Wait for another thread to finish.
   *
   * @param current_thread The thread that wants to wait.
   * @param target_thread The thread to wait for.
   *
   * @return sleeps the current_thread if the target_thread has not reached a simsync::thread_finish event.
   */
  transition join(int32_t current_thread, int32_t target_thread);

  /**
   * Signal that a thread has finished executing.
   *
   * @param thread_id The thread that has finished.
   *
   * @return wakes up a thread that was waiting from a simsync::thread_join event.
   */
  transition finish(int32_t thread_id);

  /**
   * Attempt to acquire a lock.
   *
   * @param thread_id The thread that is trying to acquire the lock.
   * @param lock_address The address the lock is located at.
   *
   * @return if the address is already locked by another thread, this thread will wait.
   */
  transition acquire(int32_t thread_id, uint64_t lock_address);

  /**
   * Release a lock.
   *
   * @param thread_id The thread that is releasing the lock.
   * @param lock_address The address the lock is located at.
   *
   * @return wakes up the next thread waiting for the lock, if any.
   */
  transition release(int32_t thread_id, uint64_t lock_address);

  /**
   * Wait for a barrier.
   *
   * @param thread_id The thread that wants to wait.
   * @param barrier_address The address the barrier is located at.
   *
   * @return wakes up all threads if the current thread is the last at the barrier, otherwise the current thread is put
   * to sleep
   */
  transition barrier_wait(int32_t thread_id, uint64_t barrier_address);

  transition condition_broadcast(uint64_t condition_address);

  transition condition_signal(uint64_t condition_address);

  transition condition_wait(int32_t thread_id, uint64_t condition_address);

private:
  enum class condition_type { spsc, spmc, mpsc, mpmc, unknown };

  struct condition_info {
    condition_type type = condition_type::unknown;
    std::set<int32_t> broadcasters;
    std::set<int32_t> signalers;
    std::set<int32_t> waiters;
  };

  // barrier objects
  std::map<uint64_t, size_t> m_barriers;
  // data on condition variables
  std::map<uint64_t, condition_info> m_condition_info;

  // threads waiting to start
  std::deque<int32_t> m_start_queue;
  // threads waiting on another thread
  std::map<int32_t, int32_t> m_join_queue;
  // for each barrier: threads waiting on that barrier
  std::map<uint64_t, std::set<int32_t>> m_barrier_queue;
  // for each lock: threads waiting on that lock in order of acquire
  std::map<uint64_t, std::deque<int32_t>> m_lock_queue;
  // for each thread: locks that thread has acquired
  std::map<int32_t, std::deque<uint64_t>> m_thread_locks;
  // for each condition variable: threads waiting on that condition variable
  std::map<uint64_t, std::deque<int32_t>> m_condition_queue;
  // for each thread: locks that thread must silently re-acquire
  std::map<int32_t, std::deque<uint64_t>> m_silent_reacquire;
  // for each condition variable: approximation of production by producers
  std::map<uint64_t, size_t> m_production;

  void work(int32_t thread_id);
  void wait(int32_t thread_id);

  // threads that are now waiting on any event
  std::set<int32_t> m_working;
  // threads that are waiting on an event
  std::set<int32_t> m_waiting;
  // threads that have finished
  std::set<int32_t> m_finished;
};
}

#endif //SIMSYNC_SYNCHRONIZER_HPP
