#include "simsync/synchronization/thread_finish.hpp"

#include "simsync/synchronization/thread_model.hpp"

#include <ostream>

namespace simsync {

thread_finish::thread_finish(int32_t const thread_id, thread_model &tm) : event(thread_id, tm)
{
}

transition thread_finish::synchronize()
{
  return m_thread_model.finish(m_thread_id);
}

void thread_finish::print(std::ostream &stream) const
{
  stream << "finished";
}
}
