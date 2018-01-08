#include "simsync/core_type.hpp"

namespace simsync {
void core_type::add_cpi_rate(int32_t const thread_id, double cpi_rate)
{
  auto emplaced = m_cpi_rates.emplace(thread_id, cpi_rate);

  if(!emplaced.second) {
    throw std::runtime_error("Error: attempting to overwrite CPI for thread.");
  }
}

void core_type::add_frequency(int32_t level, int64_t frequency)
{
  m_frequencies.emplace(level, frequency);
}

double core_type::get_cpi(int32_t thread_id) const
{
  auto const cpi_it = m_cpi_rates.find(thread_id);
  if(cpi_it != m_cpi_rates.end()) {
    return cpi_it->second;
  }

  throw std::runtime_error("Error: could not find CPI for thread.");
}

int64_t core_type::get_frequency(int32_t id) const
{
  return m_frequencies.at(id);
}
}
