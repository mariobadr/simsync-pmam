#include "simsync/synchronization/thread_start.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {

thread_start::thread_start(int32_t const thread_id, thread_model &tm) : event(thread_id, tm)
{
}

transition thread_start::synchronize()
{
  return m_thread_model.start(m_thread_id);
}

void thread_start::print(std::ostream &stream) const
{
  stream << "started";
}
}
