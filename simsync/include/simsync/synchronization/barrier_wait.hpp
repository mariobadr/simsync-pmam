#ifndef SIMSYNC_BARRIER_WAIT_HPP
#define SIMSYNC_BARRIER_WAIT_HPP

#include <simsync/synchronization/event.hpp>

namespace simsync {
class barrier_wait : public event {
public:
  explicit barrier_wait(int32_t thread_id, thread_model &tm, uint64_t barrier_address);

  transition synchronize() override;

private:
  uint64_t const m_barrier_address;

  void print(std::ostream &stream) const override;
};
}

#endif //SIMSYNC_BARRIER_WAIT_HPP
