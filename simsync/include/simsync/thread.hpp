#ifndef SIMSYNC_THREAD_HPP
#define SIMSYNC_THREAD_HPP

#include <simsync/synchronization/event.hpp>

#include <cstdint>
#include <deque>
#include <memory>

namespace simsync {

using event_ptr = std::unique_ptr<event>;

/**
 * A thread model.
 *
 * Represents a thread as a sequence of simsync::event. Between events is the computation necessary to reach them, which
 * is represented as an instruction count.
 */
class thread {
public:
  explicit thread(int32_t thread_id);

  /**
   * Add a new event after a certain amount of computation.
   *
   * @param computation Number of instructions leading up to the event.
   * @param new_event The new event to add.
   */
  void add_event(uint64_t computation, event_ptr new_event);

  /**
   * @return The number of events and computations in this thread.
   */
  size_t size() const;

  /**
   * Get the computation at a certain index.
   *
   * @param index The computation desired.
   *
   * @return The instruction count for the computation.
   */
  uint64_t get_computation(size_t index) const;

  /**
   * Get the event at a certain index.
   *
   * @param index The event desired.
   *
   * @return The synchronization event.
   */
  event * get_event(size_t index) const;

private:
  int32_t const m_thread_id;

  std::deque<uint64_t> m_computations;

  std::deque<event_ptr> m_events;
};
}

#endif //SIMSYNC_THREAD_HPP
