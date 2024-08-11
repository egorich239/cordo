#include <cordo/cordo.hh>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <type_traits>

namespace cordo_json {

struct to_json_core_t final {
 private:
  using json = nlohmann::json;
  template <cordo::cstring K, cordo::algo Algo, cordo::mirror_struct M>
  std::pair<std::string, json> make_struct_field(cordo::key_t<K> k,
                                                 const Algo& rec, M&& m) const {
    json j;
    m[k] | cordo::piped(rec, j);  // TODO: assert_value
    return {std::string(k()()), j};
  }

  template <typename... K, cordo::algo Algo, cordo::mirror_struct M>
  nlohmann::json make_struct_map(cordo::types_t<K...>, const Algo& rec,
                                 M&& m) const {
    return json::object({make_struct_field(K{}, rec, m)...});
  }

 public:
  template <typename..., cordo::mirror_primitive M>
  void operator()(M&& m, nlohmann::json& j) const {
    using nlohmann::to_json;
    to_json(j, m.v());
  }
  template <typename..., cordo::algo Algo, cordo::mirror_option M>
  void operator()(const Algo& rec, M&& m, nlohmann::json& j) const {
    if (m.has_value()) {
      // TODO: assert success.
      m.unwrap() | cordo::piped(rec, j);
    } else {
      j = nullptr;
    }
  }
  template <typename..., cordo::algo Algo, cordo::mirror_struct M>
  void operator()(const Algo& rec, M&& m, nlohmann::json& j) const {
    using traits = typename std::remove_cvref_t<M>::traits;
    constexpr auto keys = cordo::mirror_traits_subscript_keys(traits{});
    j = make_struct_map(keys, rec, (M&&)m);
  }
};
struct from_json_core_t final {};

constexpr inline cordo::algo_t<to_json_core_t{}> to_json{};
constexpr inline cordo::algo_t<from_json_core_t{}> from_json{};

}  // namespace cordo_json