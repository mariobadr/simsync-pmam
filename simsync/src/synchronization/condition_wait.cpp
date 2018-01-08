#include "simsync/synchronization/condition_wait.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {

condition_wait::condition_wait(int32_t thread_id, thread_model &tm, uint64_t condition_address)
    : event(thread_id, tm), m_condition_address(condition_address)
{
}

transition condition_wait::synchronize()
{
  return m_thread_model.condition_wait(m_thread_id, m_condition_address);
}

void condition_wait::print(std::ostream &stream) const
{
  stream << "cond_wait(" << m_condition_address << ")";
}
}
