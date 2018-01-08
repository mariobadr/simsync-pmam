#ifndef SIMSYNC_LOCK_RELEASE_HPP
#define SIMSYNC_LOCK_RELEASE_HPP

#include <simsync/synchronization/event.hpp>

namespace simsync {
class lock_release : public event {
public:
  explicit lock_release(int32_t thread_id, thread_model &tm, uint64_t lock_address);

  transition synchronize() override;

private:
  uint64_t const m_lock_address;

  void print(std::ostream &stream) const override;
};
}

#endif //SIMSYNC_LOCK_RELEASE_HPP
