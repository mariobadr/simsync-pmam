#ifndef SIMSYNC_REPORT_HPP
#define SIMSYNC_REPORT_HPP

#include <chrono>
#include <fstream>

namespace simsync {
class event;

class report {
public:
  explicit report(std::string const &output_file) : m_stream(output_file)
  {
  }

  virtual ~report() = default;

  virtual void update(std::chrono::nanoseconds current_time, event *e) = 0;

protected:
  std::ofstream m_stream;
};
}

#endif //SIMSYNC_REPORT_HPP
