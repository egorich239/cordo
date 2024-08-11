#include <cordo/cordo.hh>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <type_traits>

namespace cordo_json {
namespace cordo_json_internal {

struct to_json_core_t final {
  template <typename..., cordo::mirror_primitive M>
  void operator()(M&& m, nlohmann::json& j) const {
    using nlohmann::to_json;
    to_json(j, m.v());
  }
};

constexpr inline cordo::algo_t<cordo::cpo_v<to_json_core_t{}, to_json_core_t>>
    to_json{};

template <typename..., cordo::mirror_option M>
void customize(cordo::hook_t<to_json>, M&& m, nlohmann::json& j) {
  if (m.has_value()) {
    // TODO: assert success.
    m.unwrap() | cordo::piped(to_json, j);
  } else {
    j = nullptr;
  }
}

inline constexpr struct make_struct_map_fn {
 private:
  using json = nlohmann::json;
  template <cordo::cstring K, cordo::mirror_struct M>
  std::pair<std::string, json> make_struct_field(cordo::key_t<K> k,
                                                 M&& m) const {
    json j;
    m[k] | cordo::piped(cordo_json_internal::to_json, j);  // TODO: assert_value
    return {std::string(k()()), j};
  }

 public:
  template <typename... K, cordo::mirror_struct M>
  json operator()(cordo::types_t<K...>, M&& m) const {
    return json::object({make_struct_field(K{}, m)...});
  }
} make_struct_map{};

template <typename..., cordo::mirror_struct M>
void customize(cordo::hook_t<to_json>, M&& m, nlohmann::json& j) {
  using traits = typename std::remove_cvref_t<M>::traits;
  constexpr auto keys = cordo::mirror_traits_subscript_keys(traits{});
  j = make_struct_map(keys, (M&&)m);
}

}  // namespace cordo_json_internal

using cordo_json_internal::to_json;

}  // namespace cordo_json