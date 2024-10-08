#pragma once

#include <climits>
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/core/reflect.hh"
#include "cordo/impl/mirror.hh"

namespace cordo {
namespace cordo_internal_mirror {

inline constexpr struct {
  // TODO: tighten the boundaries, it should be kv_t<>...
  template <auto... Kv>
  constexpr auto operator()(::cordo::values_t<Kv...>) const noexcept {
    constexpr ::cordo::types_t<decltype(decltype(Kv){}.key())...> result{};
    static_assert(::cordo::meta.unique(result));
    return result;
  }
} mirror_struct_eval_keys{};

template <typename T, typename Map>
struct mirror_struct_traits final {
  using t = T;
  using rep = T&;

  using name =
      ::cordo::make_key<::cordo_internal_reflect::reflect.type_base_name(
          ::cordo::tag_t<std::remove_cv_t<T>>{})>;

  using subscript_map = Map;
  using subscript_keys = decltype(mirror_struct_eval_keys(Map{}));
};

struct mirror_struct_access_core_t final {
  template <typename T, typename F>
  constexpr auto operator()(T& s, F T::*f) const noexcept -> decltype(s.*f) {
    return s.*f;
  }
  template <typename T, typename F>
  constexpr auto operator()(const T& s,
                            F T::*f) const noexcept -> decltype(s.*f) {
    return s.*f;
  }
};
inline constexpr ::cordo::algo_t<mirror_struct_access_core_t{}>
    mirror_struct_access;

template <typename T, typename Map, auto K>
constexpr decltype(auto) customize(hook_t<mirror_subscript_key>,
                                   mirror_struct_traits<T, Map>, auto&& core,
                                   ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(
        core |
        cordo::piped(make_mirror_impl,
                     mirror_struct_access(
                         ((decltype(core)&&)core).value,
                         ::cordo::kv_lookup(
                             typename decltype(core.traits())::subscript_map{},
                             k))));

template <typename T, typename Map>
constexpr auto customize(hook_t<mirror_traits_of_const>,
                         mirror_struct_traits<T, Map>) noexcept {
  return mirror_struct_traits<const T, Map>{};
}

}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_struct_traits;

template <typename M>
concept mirror_struct = requires(typename std::remove_cvref_t<M>::traits t) {
  requires is_mirror<M>;
  { mirror_struct_traits{t} } -> std::same_as<decltype(t)>;
};

}  // namespace cordo