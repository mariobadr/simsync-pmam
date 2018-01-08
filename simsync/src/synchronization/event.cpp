#include "simsync/synchronization/event.hpp"

namespace simsync {

event::event(int32_t const thread_id, thread_model &tm) : m_thread_id(thread_id), m_thread_model(tm)
{
}
}
