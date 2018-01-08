#include "simsync/synchronization/thread_join.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {
thread_join::thread_join(int32_t thread_id, thread_model &tm, int32_t target_thread)
    : event(thread_id, tm), m_target_thread(target_thread)
{
}

transition thread_join::synchronize()
{
  return m_thread_model.join(m_thread_id, m_target_thread);
}

void thread_join::print(std::ostream &stream) const
{
  stream << "join(" << m_target_thread << ")";
}
}
