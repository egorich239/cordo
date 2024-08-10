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
struct mirror_struct final {
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
  constexpr auto operator()(const ::cordo::algo<mirror_struct_access_core_t>&,
                            T& s, F T::*f) const noexcept -> decltype(s.*f) {
    return s.*f;
  }
  template <typename T, typename F>
  constexpr auto operator()(const ::cordo::algo<mirror_struct_access_core_t>&,
                            const T& s,
                            F T::*f) const noexcept -> decltype(s.*f) {
    return s.*f;
  }
};
inline constexpr ::cordo::algo<mirror_struct_access_core_t>
    mirror_struct_access;

template <typename T, typename Map, typename EH, auto K>
constexpr decltype(auto) customize(mirror_subscript_key_core_t, const auto&,
                                   mirror_core<mirror_struct<T, Map>, EH>& core,
                                   ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(::cordo::mirror.core(
        mirror_struct_access(
            core.value,
            ::cordo::kv_lookup(
                typename decltype(core.traits())::subscript_map{}, k)),
        EH{}));

template <typename T, typename Map, typename EH, auto K>
constexpr decltype(auto) customize(
    mirror_subscript_key_core_t, const auto&,
    const mirror_core<mirror_struct<T, Map>, EH>& core, ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(::cordo::mirror.core(
        mirror_struct_access(
            core.value,
            ::cordo::kv_lookup(
                typename decltype(core.traits())::subscript_map{}, k)),
        EH{}));

template <typename T, typename Map>
constexpr auto customize(mirror_traits_of_const_core_t, const auto&,
                         mirror_struct<T, Map>) noexcept {
  return mirror_struct<const T, Map>{};
}

}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_struct;
}  // namespace cordo