#include "simsync/synchronization/lock_release.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {

lock_release::lock_release(int32_t thread_id, thread_model &tm, uint64_t lock_address)
    : event(thread_id, tm), m_lock_address(lock_address)
{
}

transition lock_release::synchronize()
{
  return m_thread_model.release(m_thread_id, m_lock_address);
}

void lock_release::print(std::ostream &stream) const
{
  stream << "release(" << m_lock_address << ")";
}
}
