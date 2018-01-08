#ifndef SIMSYNC_EVENT_TRACE_HPP
#define SIMSYNC_EVENT_TRACE_HPP

#include <simsync/reports/report.hpp>
#include <simsync/synchronization/event.hpp>

#include <fstream>

namespace simsync {

class event_trace : public report {
public:
  explicit event_trace(std::string const &output_file) : report(output_file)
  {
  }

  void update(std::chrono::nanoseconds const current_time, event *e) override
  {
    m_stream << e->get_thread_id() << " " << current_time.count() << " " << *e << std::endl;
  }
};
}

#endif //SIMSYNC_EVENT_TRACE_HPP
