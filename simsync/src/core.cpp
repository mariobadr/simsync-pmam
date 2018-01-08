#include "simsync/core.hpp"

#include "simsync/core_type.hpp"

namespace simsync {

core::core(core_type const &type) : m_type(type), m_frequency(type.get_frequency(0))
{
}

void core::scale_frequency(int32_t level)
{
  m_frequency = m_type.get_frequency(level);
}

double core::get_cpi(int32_t thread_id) const
{
  return m_type.get_cpi(thread_id);
}
}
