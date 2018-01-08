#ifndef SIMSYNC_SYSTEM_HPP
#define SIMSYNC_SYSTEM_HPP

#include <cstdint>
#include <deque>
#include <map>
#include <set>

namespace simsync {
class architecture;
class core;

/**
 * An operating system model.
 */
class system {
public:
  /**
   * Construct an operating system to manage an simsync::architecture.
   *
   * @param config_file The configuration input, which is expected to be valid.
   * @param arch The architecture to manage.
   */
  explicit system(const std::string &config_file, architecture &arch);

  /**
   * @return The set of currently executing threads.
   */
  std::set<int32_t> const &executing_threads() const
  {
    return m_executing_threads;
  }

  /**
   * @return the set of currently sleeping threads.
   */
  std::set<int32_t> const &sleeping_threads() const
  {
    return m_sleeping_threads;
  }

  /**
   * @return the set of threads waiting for a core to become available.
   */
  std::deque<int32_t> const &waiting_threads() const
  {
    return m_waiting_threads;
  }

  /**
   * Get the core the thread is running on.
   *
   * This is only valid for thread ids that are currently executing.
   *
   * @param thread_id An executing thread.
   * @return the core the thread is running on.
   */
  core const &get_thread_core(int32_t thread_id) const;

  /**
   * Schedule a thread to run on a core.
   *
   * The thread may not be immediately assigned to a core if none are available.
   *
   * @param thread_id The thread to schedule.
   */
  void schedule(int32_t thread_id);

  /**
   * Schedule many threads to run.
   *
   * @param threads The set of threads to schedule.
   */
  void schedule(std::set<int32_t> const &threads);

  /**
   * Allow the thread to sleep, removing it from execution.
   *
   * @param thread_id The thread to sleep.
   */
  void sleep(int32_t thread_id);

  /**
   * Allow many threads to sleep, removing them from execution.
   *
   * @param threads The threads to sleep.
   */
  void sleep(std::set<int32_t> const &threads);

  /**
   * Remove a thread from memory, likely because it has finished execution.
   *
   * @param thread_id The thread to erase.
   */
  void erase(int32_t thread_id);

private:
  architecture &m_architecture;

  std::deque<size_t> m_available_cores;

  std::deque<int32_t> m_waiting_threads;

  std::map<int32_t, size_t> m_thread_assignment;

  std::set<int32_t> m_executing_threads;

  std::set<int32_t> m_sleeping_threads;

  // maps threads to a static frequency level
  std::map<int32_t, int32_t> m_static_frequencies;

  void use_next_core(int32_t thread_id);

  void free_core(int32_t thread_id);

  void schedule_waiting_thread();
};
}

#endif //SIMSYNC_SYSTEM_HPP
