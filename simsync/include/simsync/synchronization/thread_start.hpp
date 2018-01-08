#ifndef SIMSYNC_THREAD_START_HPP
#define SIMSYNC_THREAD_START_HPP

#include <simsync/synchronization/event.hpp>

namespace simsync {
class thread_start : public event {
public:
  explicit thread_start(int32_t thread_id, thread_model &tm);

  transition synchronize() override;

private:
  void print(std::ostream &stream) const override;
};
}

#endif //SIMSYNC_THREAD_START_HPP
