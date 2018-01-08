#ifndef SIMSYNC_THREAD_JOIN_HPP
#define SIMSYNC_THREAD_JOIN_HPP

#include <simsync/synchronization/event.hpp>

namespace simsync {

/**
 * Wait until another thread has finished.
 */
class thread_join : public event {
public:
  /**
   * Construct a join event.
   *
   * @param thread_id The thread that wants to wait.
   * @param target_thread The thread to be waited for.
   */
  explicit thread_join(int32_t thread_id, thread_model &tm, int32_t target_thread);

  transition synchronize() override;

private:
  int32_t const m_target_thread;

  void print(std::ostream &stream) const override;
};
}

#endif //SIMSYNC_THREAD_JOIN_HPP
