#pragma once

#include <climits>
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/core/reflect.hh"
#include "cordo/impl/get.hh"
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

}  // namespace cordo_internal_mirror

namespace cordo_internal_cpo {

template <typename T, typename Traits, auto K>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::mirror_subscript_key_cpo, adl_tag, Traits, T& s,
     ::cordo::key_t<K> k),  //
    (/* TODO: simplify accessors */ ::cordo::get(
        ::cordo::make_accessor(
            ::cordo::kv_lookup(typename Traits::subscript_map{}, k)),
        s)));

}