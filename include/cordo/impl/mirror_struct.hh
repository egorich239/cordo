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

struct mirror_struct_access_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;

  template <typename T, typename F>
  constexpr auto operator()(const ::cordo::algo<mirror_struct_access_t>&, T& s,
                            F T::*f) const noexcept -> decltype(s.*f) {
    return s.*f;
  }
  template <typename T, typename F>
  constexpr auto operator()(const ::cordo::algo<mirror_struct_access_t>&,
                            const T& s, F T::*f) const noexcept
      -> decltype(s.*f) {
    return s.*f;
  }
};
inline constexpr ::cordo::algo<mirror_struct_access_t> mirror_struct_access;
}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirror_struct_access;
}  // namespace cordo

namespace cordo_internal_cpo {

template <typename T, typename Map, auto K>
constexpr decltype(auto) customize(
    decltype(::cordo::mirror_subscript_key), adl_tag,
    ::cordo::mirror_core<::cordo_internal_mirror::mirror_struct<T, Map>>& core,
    ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(::cordo::mirror.core(::cordo::mirror_struct_access(
        core.value,
        ::cordo::kv_lookup(typename decltype(core.traits())::subscript_map{},
                           k))));

template <typename T, typename Map, auto K>
constexpr decltype(auto) customize(
    decltype(::cordo::mirror_subscript_key), adl_tag,
    const ::cordo::mirror_core<::cordo_internal_mirror::mirror_struct<T, Map>>&
        core,
    ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(::cordo::mirror.core(::cordo::mirror_struct_access(
        core.value,
        ::cordo::kv_lookup(typename decltype(core.traits())::subscript_map{},
                           k))));

template <typename T, typename Map>
constexpr auto customize(
    decltype(::cordo::mirror_traits_of_const), adl_tag,
    ::cordo_internal_mirror::mirror_struct<T, Map>) noexcept {
  return ::cordo_internal_mirror::mirror_struct<const T, Map>{};
}

}  // namespace cordo_internal_cpo