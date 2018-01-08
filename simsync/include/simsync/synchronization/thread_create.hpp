#ifndef SIMSYNC_THREAD_CREATE_HPP
#define SIMSYNC_THREAD_CREATE_HPP

#include <simsync/synchronization/event.hpp>

namespace simsync {

/**
 * Create a thread.
 */
class thread_create : public event {
public:
  /**
   * Construct a thread creation event.
   *
   * @param parent_id The current thread performing the event.
   * @param future_id The ID of the thread that will be created.
   */
  explicit thread_create(int32_t parent_id, thread_model &tm, int32_t future_id);

  transition synchronize() override;

private:
  int32_t const m_future_thread_id;

  void print(std::ostream &stream) const override;
};
}

#endif //SIMSYNC_THREAD_CREATE_HPP
