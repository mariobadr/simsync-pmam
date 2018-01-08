#include "simsync/system.hpp"

#include "simsync/architecture.hpp"

#include <fstream>
#include <json.hpp>

namespace simsync {

system::system(const std::string &config_file, architecture &arch) : m_architecture(arch)
{
  auto stream = std::ifstream(config_file);
  auto input = nlohmann::json::parse(stream);

  for(auto const &frequency: input["system"]["static.frequencies"]) {
    m_static_frequencies.emplace(frequency["tid"], frequency["level"]);
  }

  for(size_t i = 0; i < m_architecture.size(); ++i) {
    m_available_cores.push_back(i);
  }
}

void system::schedule(int32_t const thread_id)
{
  auto thread_it = m_executing_threads.find(thread_id);
  if(thread_it == m_executing_threads.end()) {
    if(!m_available_cores.empty()) {
      use_next_core(thread_id);
    } else {
      // no cores available, need to wait
      m_waiting_threads.push_back(thread_id);
    }

    // thread may have been sleeping beforehand, it is now either executing or waiting for a core
    thread_it = m_sleeping_threads.find(thread_id);
    if(thread_it != m_sleeping_threads.end()) {
      m_sleeping_threads.erase(thread_it);
    }
  }
}

void system::schedule(std::set<int32_t> const &threads)
{
  for(auto const &thread_id : threads) {
    schedule(thread_id);
  }
}

void system::sleep(int32_t const thread_id)
{
  bool was_not_sleeping = false;
  std::tie(std::ignore, was_not_sleeping) = m_sleeping_threads.insert(thread_id);

  if(was_not_sleeping) {
    // thread should no longer be executing, move to sleeping
    m_executing_threads.erase(thread_id);
    m_sleeping_threads.insert(thread_id);

    free_core(thread_id);
  }

  schedule_waiting_thread();
}

void system::sleep(std::set<int32_t> const &threads)
{
  for(auto const &thread_id : threads) {
    sleep(thread_id);
  }
}

void system::erase(int32_t thread_id)
{
  auto thread_it = m_executing_threads.find(thread_id);
  if(thread_it != m_executing_threads.end()) {
    free_core(thread_id);
    m_executing_threads.erase(thread_it);
  } else {
    throw std::runtime_error("Error: erasing thread that was not executing. How did it finish?");
  }
}

core const &system::get_thread_core(int32_t const thread_id) const
{
  auto assignment_it = m_thread_assignment.find(thread_id);
  if(assignment_it != m_thread_assignment.end()) {
    return m_architecture.get_core(assignment_it->second);
  }

  throw std::runtime_error("Error: the requested thread is not assigned to a core.");
}

void system::use_next_core(int32_t const thread_id)
{
  auto const core_id = m_available_cores.front();
  m_available_cores.pop_front();

  m_thread_assignment[thread_id] = core_id;
  m_architecture.get_core(core_id).scale_frequency(m_static_frequencies[thread_id]);

  // thread should now be executing
  m_executing_threads.insert(thread_id);
}

void system::free_core(int32_t const thread_id)
{
  m_available_cores.push_back(m_thread_assignment[thread_id]);
  m_thread_assignment.erase(thread_id);

  schedule_waiting_thread();
}

void system::schedule_waiting_thread()
{
  if(!m_waiting_threads.empty() && !m_available_cores.empty()) {
    auto const next_thread = m_waiting_threads.front();
    m_waiting_threads.pop_front();

    use_next_core(next_thread);
  }
}
}
