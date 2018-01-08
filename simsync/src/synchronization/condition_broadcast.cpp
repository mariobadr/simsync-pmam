#include "simsync/synchronization/condition_broadcast.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {

condition_broadcast::condition_broadcast(int32_t thread_id, thread_model &tm, uint64_t condition_address)
    : event(thread_id, tm), m_condition_address(condition_address)
{
}

transition condition_broadcast::synchronize()
{
  return m_thread_model.condition_broadcast(m_condition_address);
}

void condition_broadcast::print(std::ostream &stream) const
{
  stream << "cond_broadcast(" << m_condition_address << ")";
}
}