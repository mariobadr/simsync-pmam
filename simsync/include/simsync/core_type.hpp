#ifndef SIMSYNC_CORE_TYPE_HPP
#define SIMSYNC_CORE_TYPE_HPP

#include <cstdint>
#include <deque>
#include <map>

namespace simsync {

/**
 * A type of core. Many cores can have the same type.
 */
class core_type {
public:
  /**
   * Assign the cycles-per-instruction for a particular thread that may run on this core.
   *
   * @param thread_id The ID of the thread.
   * @param cpi_rate The CPI rate for the thread running on this core.
   */
  void add_cpi_rate(int32_t thread_id, double cpi_rate);

  /**
   * Add a possible frequency for this core type.
   *
   * @param level The ID for the frequency level
   * @param frequency The frequency in hertz.
   */
  void add_frequency(int32_t level, int64_t frequency);

  /**
   * Get the cycles-per-instruction for a thread running on this core.
   *
   * @param thread_id The ID of the thread.
   * @return The CPI.
   */
  double get_cpi(int32_t thread_id) const;

  /**
   * Get the frequency for the specified level.
   *
   * @param id The frequency level.
   * @return The frequency in hertz.
   */
  int64_t get_frequency(int32_t id) const;

private:
  std::map<int32_t, int64_t> m_frequencies;

  // each key corresponds to a thread id
  std::map<int32_t, double> m_cpi_rates;
};
}

#endif //SIMSYNC_CORE_TYPE_HPP
