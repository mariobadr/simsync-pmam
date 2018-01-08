#include "simsync/synchronization/condition_signal.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {

condition_signal::condition_signal(int32_t thread_id, thread_model &tm, uint64_t condition_address)
    : event(thread_id, tm), m_condition_address(condition_address)
{
}

transition condition_signal::synchronize()
{
  return m_thread_model.condition_signal(m_condition_address);
}

void condition_signal::print(std::ostream &stream) const
{
  stream << "cond_signal(" << m_condition_address << ")";
}
}
