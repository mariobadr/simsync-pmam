#ifndef SIMSYNC_TIME_STACK_HPP
#define SIMSYNC_TIME_STACK_HPP

#include <simsync/reports/report.hpp>

#include <map>

namespace simsync {
class system;

class time_stack : public report {
public:
  explicit time_stack(const std::string &output_file, system const &sys);

  ~time_stack() override;

  void update(std::chrono::nanoseconds current_time, event *e) override;

private:
  struct wrapper {
    std::chrono::nanoseconds compute;
    std::chrono::nanoseconds sync;
    std::chrono::nanoseconds wait;
  };

  system const &m_system;

  std::chrono::nanoseconds m_last_time;

  std::map<int32_t, wrapper> m_wrappers;
};
}

#endif //SIMSYNC_TIME_STACK_HPP
