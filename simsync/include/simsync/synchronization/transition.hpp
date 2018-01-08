#ifndef SIMSYNC_TRANSITION_HPP
#define SIMSYNC_TRANSITION_HPP

#include <cstdint>
#include <set>

namespace simsync {

struct transition {
  std::set<int32_t> to_wake;
  std::set<int32_t> to_sleep;
  int32_t finished = -1;
};

}

#endif //SIMSYNC_TRANSITION_HPP
