#include "simsync/synchronization/thread_create.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {
thread_create::thread_create(int32_t parent_id, thread_model &tm, int32_t future_id)
    : event(parent_id, tm), m_future_thread_id(future_id)
{
}

transition thread_create::synchronize()
{
  return m_thread_model.create(m_future_thread_id);
}

void thread_create::print(std::ostream &stream) const
{
  stream << "create(" << m_future_thread_id << ")";
}
}
