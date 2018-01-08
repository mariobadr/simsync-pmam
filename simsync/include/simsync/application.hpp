#ifndef SIMSYNC_APPLICATION_HPP
#define SIMSYNC_APPLICATION_HPP

#include <simsync/thread.hpp>
#include <simsync/synchronization/thread_model.hpp>

#include <cstdint>
#include <iosfwd>
#include <map>

namespace simsync {

/**
 * An application model.
 *
 * Represents an application as a collection of simsync::thread, which synchronize through a simsync::thread_model.
 */
class application {
public:
  /**
   * Construct an application from events found in a trace.
   *
   * @param trace The trace input, which is expected to be valid.
   */
  explicit application(std::istream &trace);

  /**
   * Get one of the threads of the application.
   *
   * @param thread_id The ID of the thread to query.
   *
   * @return The thread at the specified ID.
   */
  thread const &at(int32_t thread_id) const;

  /**
   * @return All threads, indexable by thread ID.
   */
  std::map<int32_t, thread> const &threads() const;

private:
  thread_model m_thread_model;

  std::map<int32_t, thread> m_threads;
};
}

#endif //SIMSYNC_APPLICATION_HPP
