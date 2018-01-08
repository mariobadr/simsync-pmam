#ifndef SIMSYNC_THREAD_FINISH_HPP
#define SIMSYNC_THREAD_FINISH_HPP

#include <simsync/synchronization/event.hpp>

namespace simsync {

/**
 * Finish execution of a thread.
 */
class thread_finish : public event {
public:
  /**
   * Construct an event to signal execution has finished.
   *
   * @param thread_id The thread that has finished.
   */
  explicit thread_finish(int32_t thread_id, thread_model &tm);

  transition synchronize() override;

private:
  void print(std::ostream &stream) const override;
};
}

#endif //SIMSYNC_THREAD_FINISH_HPP
