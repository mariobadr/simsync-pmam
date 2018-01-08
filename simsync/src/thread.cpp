#include "simsync/thread.hpp"

namespace simsync {
thread::thread(int32_t thread_id) : m_thread_id(thread_id)
{
}

void thread::add_event(uint64_t computation, event_ptr new_event)
{
  m_computations.push_back(computation);
  m_events.push_back(std::move(new_event));
}
size_t thread::size() const
{
  return m_computations.size();
}
uint64_t thread::get_computation(size_t const index) const
{
  return m_computations[index];
}
event * thread::get_event(size_t index) const
{
  return m_events[index].get();
}
}
