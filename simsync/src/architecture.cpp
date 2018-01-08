#include "simsync/architecture.hpp"

#include <fstream>
#include <json.hpp>

namespace simsync {
architecture::architecture(const std::string &config_file)
{
  auto stream = std::ifstream(config_file);
  auto input = nlohmann::json::parse(stream);

  for(auto const &core_type_config : input["architecture"]["core.types"]) {
    core_type new_core_type;

    for(auto const &thread : core_type_config["threads"]) {
      int32_t const thread_id = thread["tid"];
      double const cpi_rate = thread["cpi.rate"];

      new_core_type.add_cpi_rate(thread_id, cpi_rate);
    }

    for(auto const &level: core_type_config["frequency.levels"]) {
      new_core_type.add_frequency(level["id"], level["frequency"]);
    }

    int32_t const core_type_id = core_type_config["id"];
    m_types.emplace(core_type_id, std::move(new_core_type));
  }

  std::deque<int32_t> cores = input["architecture"]["cores"];
  for(auto const core_type_id : cores) {
    m_cores.emplace_back(m_types.at(core_type_id));
  }
}

size_t architecture::size() const
{
  return m_cores.size();
}

core &architecture::get_core(size_t const index)
{
  return m_cores[index];
}
}
