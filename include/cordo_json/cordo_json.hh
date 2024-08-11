#include "cordo/cordo.hh"

namespace cordo_json {

struct to_json_core_t final {
  template <typename..., cordo::algo Algo, typename I>
  auto customize(Algo rec, const cordo::mirror_t<I>& m) const {
    return rec(typename cordo::mirror_t<I>::traits{}, m);
  }
};
struct from_json_core_t final {};

constexpr inline cordo::algo_t<to_json_core_t{}> to_json{};
constexpr inline cordo::algo_t<from_json_core_t{}> from_json{};

}  // namespace cordo_json