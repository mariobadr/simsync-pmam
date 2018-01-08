#ifndef SIMSYNC_CRITICALITY_STACK_HPP
#define SIMSYNC_CRITICALITY_STACK_HPP

#include <simsync/reports/report.hpp>

#include <map>

namespace simsync {
class system;

class criticality_stack : public report {
public:
  explicit criticality_stack(std::string const &output_file, const system &system);

  ~criticality_stack() override;

  void update(std::chrono::nanoseconds current_time, event *e) override;

private:
  system const &m_system;

  std::chrono::nanoseconds m_last_time;

  std::map<int32_t, int64_t> m_criticality;
};
}

#endif //SIMSYNC_CRITICALITY_STACK_HPP
