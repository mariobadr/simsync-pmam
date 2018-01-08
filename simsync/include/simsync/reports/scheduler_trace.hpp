#ifndef SIMSYNC_SCHEDULER_TRACE_HPP
#define SIMSYNC_SCHEDULER_TRACE_HPP

#include <simsync/system.hpp>
#include <simsync/reports/report.hpp>
#include <simsync/synchronization/event.hpp>

#include <string>

namespace simsync {

class scheduler_trace : public report {
public:
  explicit scheduler_trace(std::string const &output_file, system const &sys)
      : report(output_file), m_system(sys)
  {
  }

  void update(std::chrono::nanoseconds const current_time, event *e) override
  {
    m_stream << e->get_thread_id() << " " << current_time.count() << " " << *e << " ";
    print_threads("running", m_system.executing_threads());
    print_threads("sleeping", m_system.sleeping_threads());
    print_threads("waiting", m_system.waiting_threads());

    m_stream << std::endl;
  }

private:
  system const &m_system;

  template <typename Container>
  void print_threads(std::string const &name, Container const &threads)
  {
    m_stream << name << "(" << threads.size() << ") = {";
    for(auto const &t : threads) {
      m_stream << t << ",";
    }
    m_stream << "} ";
  }
};
}

#endif //SIMSYNC_SCHEDULER_TRACE_HPP
