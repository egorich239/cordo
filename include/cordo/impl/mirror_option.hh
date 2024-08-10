#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/core/pipe.hh"
#include "cordo/impl/mirror.hh"

namespace cordo {
namespace cordo_internal_mirror {

template <typename T, typename I, typename Rep = T&>
struct mirror_option final {
  using t = T;
  using rep = Rep;
  using inner = I;

  using name = ::cordo::null_t;
};

template <typename T, typename I, typename Rep>
constexpr auto customize(mirror_unwrap_core_t, mirror_option<T, I, Rep>,
                         auto&& core)
    CORDO_INTERNAL_ALIAS_(
        core.value
            ? (core |
               cordo::piped(make_mirror_impl, *((decltype(core)&&)core).value) |
               cordo::piped(make_mirror_result))
            : (core | cordo::piped(make_mirror_error,
                                   ::cordo::mirror_error::INVALID_UNWRAP)));

template <typename T, typename I, typename Rep>
constexpr auto customize(mirror_traits_subscript_keys_core_t, const auto& rec,
                         mirror_option<T, I, Rep> t)
    CORDO_INTERNAL_ALIAS_(rec(::cordo::mirror.t(::cordo::tag_t<I>{})));

template <typename T, typename I, typename Rep, auto K>
constexpr decltype(auto) customize(mirror_subscript_key_core_t, const auto& rec,
                                   mirror_option<T, I, Rep>, auto&& core,
                                   ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(mirror_impl_apply((decltype(core)&&)core,
                                             mirror_unwrap) |
                           cordo::piped(mirror_impl_apply, rec, k));

template <typename T>
constexpr auto customize(mirror_traits_ctor_core_t,
                         ::cordo::tag_t<std::unique_ptr<T>>) noexcept {
  return mirror_option<std::unique_ptr<T>, T&>{};
}

template <typename T, typename I>
constexpr auto customize(mirror_traits_of_const_core_t,
                         mirror_option<T, I, T&>) noexcept {
  return mirror_option<const T, const I, const T&>{};
}

}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_option;
}  // namespace cordo