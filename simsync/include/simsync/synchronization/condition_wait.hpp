#ifndef SIMSYNC_CONDITION_WAIT_HPP
#define SIMSYNC_CONDITION_WAIT_HPP

#include <simsync/synchronization/event.hpp>

namespace simsync {
class condition_wait : public event {
public:
  explicit condition_wait(int32_t thread_id, thread_model &tm, uint64_t condition_address);

  transition synchronize() override;

private:
  void print(std::ostream &stream) const override;

  uint64_t const m_condition_address;
};
}

#endif //SIMSYNC_CONDITION_WAIT_HPP
