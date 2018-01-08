#include "simsync/synchronization/lock_acquire.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {

lock_acquire::lock_acquire(int32_t thread_id, thread_model &tm, uint64_t lock_address)
    : event(thread_id, tm), m_lock_address(lock_address)
{
}

transition lock_acquire::synchronize()
{
  return m_thread_model.acquire(m_thread_id, m_lock_address);
}

void lock_acquire::print(std::ostream &stream) const
{
  stream << "acquire(" << m_lock_address << ")";
}
}
