#include "simsync/application.hpp"

#include "simsync/synchronization/barrier_wait.hpp"
#include "simsync/synchronization/condition_broadcast.hpp"
#include "simsync/synchronization/condition_signal.hpp"
#include "simsync/synchronization/condition_wait.hpp"
#include "simsync/synchronization/lock_acquire.hpp"
#include "simsync/synchronization/lock_release.hpp"
#include "simsync/synchronization/thread_create.hpp"
#include "simsync/synchronization/thread_join.hpp"
#include "simsync/synchronization/thread_start.hpp"
#include "simsync/synchronization/thread_finish.hpp"

#include <istream>
#include <sstream>

namespace simsync {

template <class Container, class Key, class... Args>
typename Container::iterator find_or_emplace(Container &container, Key const &key, Args &&... args)
{
  auto iterator = container.find(key);
  if(iterator == container.end()) {
    std::tie(iterator, std::ignore) = container.emplace(key, std::forward<Args>(args)...);
  }

  return iterator;
}

// what pthread_t typically is in the pthreads library
using pthread_t = unsigned long int;

struct trace_row {
  int32_t thread_id = -1;
  std::string call = "";
  pthread_t handle = 0;
  uint64_t object = 0;
  size_t barrier_count = 0;
  uint64_t instruction_count = 0;

  friend std::istream &operator>>(std::istream &stream, trace_row &row)
  {
    stream >> row.thread_id;
    stream >> row.call;

    uint64_t call_location;
    stream >> call_location;

    if(row.call == "pthread_create" || row.call == "pthread_join") {
      stream >> row.handle;
    } else {
      stream >> row.object;
    }

    if(row.call == "pthread_barrier_init") {
      stream >> row.barrier_count;
    }

    stream >> row.instruction_count;

    return stream;
  }
};

event_ptr create_event(thread_model &tm, trace_row const &row)
{
  static std::set<std::string> lock_calls = {
      "pthread_mutex_lock", "pthread_mutex_timedlock", "pthread_mutex_trylock",
      "pthread_rwlock_wrlock", "pthread_rwlock_timedwrlock", "pthread_rwlock_trywrlock",
      "pthread_rwlock_rdlock", "pthread_rwlock_timedrdlock", "pthread_rwlock_tryrdlock",
      "pthread_spin_lock", "pthread_spin_trylock",
  };

  static std::set<std::string> unlock_calls = {
      "pthread_mutex_unlock", "pthread_rwlock_unlock", "pthread_spin_unlock",
  };

  static std::map<pthread_t, int32_t> handles;

  if(lock_calls.find(row.call) != lock_calls.end()) {
    return std::make_unique<lock_acquire>(row.thread_id, tm, row.object);
  }

  if(unlock_calls.find(row.call) != unlock_calls.end()) {
    return std::make_unique<lock_release>(row.thread_id, tm, row.object);
  }

  if(row.call == "pthread_barrier_init") {
    tm.add_barrier(row.object, row.barrier_count);
    return nullptr;
  }

  if(row.call == "pthread_barrier_wait") {
    return std::make_unique<barrier_wait>(row.thread_id, tm, row.object);
  }

  if(row.call == "pthread_cond_init") {
    return nullptr;
  }

  if(row.call == "pthread_cond_broadcast") {
    tm.approximate_broadcast(row.thread_id, row.object);
    return std::make_unique<condition_broadcast>(row.thread_id, tm, row.object);
  }

  if(row.call == "pthread_cond_signal") {
    tm.approximate_signal(row.thread_id, row.object);
    return std::make_unique<condition_signal>(row.thread_id, tm, row.object);
  }

  if(row.call == "pthread_cond_wait") {
    tm.approximate_wait(row.thread_id, row.object);
    return std::make_unique<condition_wait>(row.thread_id, tm, row.object);
  }

  if(row.call == "pthread_create") {
    // we need to associate pthread_t handles with thread IDs
    static int32_t next_create_id = 0;
    next_create_id++;

    bool was_inserted = false;
    std::map<pthread_t, int32_t>::iterator handle;
    std::tie(handle, was_inserted) = handles.emplace(row.handle, next_create_id);
    if(!was_inserted) {
      // This can happen if a thread finishes and another pthread_create call occurs.
      // For now, assume that a join call occurs before the pthread_create, so we will
      // just overwrite the thread ID for this handle.
      handle->second = next_create_id;
    }

    return std::make_unique<thread_create>(row.thread_id, tm, next_create_id);
  }

  if(row.call == "pthread_join") {
    auto const &find_join_target = handles.find(row.handle);

    return std::make_unique<thread_join>(row.thread_id, tm, find_join_target->second);
  }

  if(row.call == "thread_start") {
    return std::make_unique<thread_start>(row.thread_id, tm);
  }

  if(row.call == "thread_finish") {
    return std::make_unique<thread_finish>(row.thread_id, tm);
  }

  return nullptr;
}

uint64_t create_work(std::deque<uint64_t> &thread_instructions)
{
  auto const &last = thread_instructions.front();
  auto const &current = thread_instructions.back();
  thread_instructions.pop_front();

  return current - last;
}

application::application(std::istream &trace)
{
  std::map<int32_t, std::deque<uint64_t>> icounts;
  std::string line;

  // read up to the first empty line (i.e., not EOF)
  while(std::getline(trace, line) && !line.empty()) {
    std::istringstream line_stream(line);
    trace_row row;

    if(line_stream >> row) {
      auto thread_it = find_or_emplace(m_threads, row.thread_id, thread(row.thread_id));
      auto instructions_it = find_or_emplace(icounts, row.thread_id, std::deque<uint64_t>{});

      auto e = create_event(m_thread_model, row);
      if(e != nullptr) {
        instructions_it->second.push_back(row.instruction_count);

        uint64_t instruction_count = 0;
        if(instructions_it->second.size() == 2) {
          instruction_count = create_work(instructions_it->second);
        }

        thread_it->second.add_event(instruction_count, std::move(e));
      }
    }
  }

  m_thread_model.classify_condition_variables();
}

thread const &application::at(int32_t thread_id) const
{
  auto thread_it = m_threads.find(thread_id);
  if(thread_it != m_threads.end()) {
    return thread_it->second;
  }

  throw std::runtime_error("Error: could not find thread in application");
}

std::map<int32_t, thread> const &application::threads() const
{
  return m_threads;
}
}
