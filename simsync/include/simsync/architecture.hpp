#ifndef SIMSYNC_ARCHITECTURE_HPP
#define SIMSYNC_ARCHITECTURE_HPP

#include <simsync/core.hpp>
#include <simsync/core_type.hpp>

#include <cstdint>
#include <deque>

namespace simsync {

/**
 * An architecture model.
 *
 * Represents an application as a collection of simsync::core.
 */
class architecture {
public:
  /**
   * Construct an architecture based on a JSON configuration.
   *
   * @param config_file The configuration file, which is expected to be valid.
   */
  explicit architecture(const std::string &config_file);

  /**
   * @return The number of cores in this architecture.
   */
  size_t size() const;

  /**
   * Get the core residing at the index.
   *
   * @param index The desired core.
   *
   * @return The core model.
   */
  core &get_core(size_t index);

private:
  // each key corresponds to a core id
  std::deque<core> m_cores;

  std::map<int32_t, core_type> m_types;
};
}

#endif //SIMSYNC_ARCHITECTURE_HPP
