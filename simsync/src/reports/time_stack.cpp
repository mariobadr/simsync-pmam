#include "simsync/reports/time_stack.hpp"

#include "simsync/system.hpp"

namespace simsync {

time_stack::time_stack(const std::string &output_file, system const &sys)
    : report(output_file), m_system(sys), m_last_time(0)
{
}

time_stack::~time_stack()
{
  m_stream << "thread,computation,synchronization,waiting,total\n";
  for(auto const &w : m_wrappers) {
    m_stream << w.first << ",";
    m_stream << w.second.compute.count() << ",";
    m_stream << w.second.sync.count() << ",";
    m_stream << w.second.wait.count() << ",";
    m_stream << (w.second.compute.count() + w.second.sync.count() + w.second.wait.count()) << "\n";
  }
}

void time_stack::update(std::chrono::nanoseconds current_time, event *)
{
  auto const delta_time = current_time - m_last_time;
  m_last_time = current_time;

  for(auto const &t : m_system.executing_threads()) {
    m_wrappers[t].compute += delta_time;
  }

  for(auto const &t : m_system.sleeping_threads()) {
    m_wrappers[t].sync += delta_time;
  }

  for(auto const &t : m_system.waiting_threads()) {
    m_wrappers[t].wait += delta_time;
  }
}
}
