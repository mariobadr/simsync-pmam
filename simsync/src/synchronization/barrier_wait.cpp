#include "simsync/synchronization/barrier_wait.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {
barrier_wait::barrier_wait(int32_t thread_id, thread_model &tm, uint64_t barrier_address)
    : event(thread_id, tm), m_barrier_address(barrier_address)
{
}

transition barrier_wait::synchronize()
{
  return m_thread_model.barrier_wait(m_thread_id, m_barrier_address);
}

void barrier_wait::print(std::ostream &stream) const
{
  stream << "barrier(" << m_barrier_address << ")";
}
}
