#ifndef SIMSYNC_CORE_HPP
#define SIMSYNC_CORE_HPP

#include <cstdint>
#include <map>

namespace simsync {
class core_type;

/**
 * A logical core model.
 *
 * Represents a logical core from a CPU, which is modelled using cycles-per-instruction (CPI) and frequency. A CPI is
 * required for each thread that can run on this core.
 */
class core {
public:
  /**
   * Construct a core.
   *
   * @param initial_frequency The initial frequency the core should operate at.
   */
  explicit core(core_type const &type);

  /**
   * Dynamically scale the frequency of this core.
   *
   * @param level The frequency level for this core's type.
   */
  void scale_frequency(int32_t level);

  /**
   * @return The current operating frequency.
   */
  int64_t frequency() const
  {
    return m_frequency;
  }

  /**
   * Get the cycles-per-instruction for a thread running on this core.
   *
   * @param thread_id The ID of the thread.
   * @return The CPI.
   */
  double get_cpi(int32_t thread_id) const;

private:
  core_type const &m_type;

  int64_t m_frequency;
};
}

#endif //SIMSYNC_CORE_HPP
