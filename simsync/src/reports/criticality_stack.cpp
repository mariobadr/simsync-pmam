#include "simsync/reports/criticality_stack.hpp"

#include "simsync/system.hpp"

namespace simsync {

criticality_stack::criticality_stack(std::string const &output_file, const system &system)
    : report(output_file), m_system(system), m_last_time(0)
{
}

criticality_stack::~criticality_stack()
{
  m_stream << "thread,criticality\n";
  for(auto const &criticality : m_criticality) {
    m_stream << criticality.first << "," << criticality.second << "\n";
  }
}

void criticality_stack::update(std::chrono::nanoseconds current_time, event *)
{
  auto const delta_time = current_time - m_last_time;
  m_last_time = current_time;

  auto const total_waiting = m_system.waiting_threads().size() + m_system.sleeping_threads().size();
  auto const total_running = m_system.executing_threads().size();

  // if only one thread is running, it should not impact their criticality
  if(total_running > 1 && total_waiting > 0) {
    for(auto const &t : m_system.executing_threads()) {
      m_criticality[t] += delta_time.count() / static_cast<int64_t>(total_waiting);
    }
  }
}
}
