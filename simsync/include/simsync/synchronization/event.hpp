#ifndef SIMSYNC_EVENT_HPP
#define SIMSYNC_EVENT_HPP

#include <cstdint>
#include <iosfwd>

namespace simsync {
class thread_model;
class transition;

/**
 * A synchronization event.
 */
class event {
public:
  /**
   * Construct a synchronization event.
   *
   * @param thread_id The thread this event belongs to.
   */
  explicit event(int32_t thread_id, thread_model &tm);

  /**
   * Destructor.
   */
  virtual ~event() = default;

  /**
   * Perform the appropriate synchronization with the simsync::thread_model.
   *
   * @param tm The thread model to use.
   *
   * @return The resulting thread transitions based on the event and the thread model's state.
   */
  virtual transition synchronize() = 0;

  /**
   * @return Get the thread ID that this event belongs to.
   */
  int32_t get_thread_id() const
  {
    return m_thread_id;
  }

  /**
   * Output the synchronization event to the stream.
   *
   * @param stream An output stream.
   * @param rhs The event object.
   *
   * @return A modified output stream.
   */
  friend std::ostream &operator<<(std::ostream &stream, event const &rhs)
  {
    rhs.print(stream);
    return stream;
  }

protected:
  int32_t const m_thread_id;

  thread_model &m_thread_model;

  virtual void print(std::ostream &stream) const = 0;
};
}

#endif //SIMSYNC_EVENT_HPP
