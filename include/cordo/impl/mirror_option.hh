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
#include "cordo/impl/mirror.hh"

namespace cordo_internal_mirror {

template <typename T, typename I, typename Rep = T&>
struct mirror_option final {
  using t = T;
  using rep = Rep;
  using inner = I;

  using name = ::cordo::null_t;
};

}  // namespace cordo_internal_mirror

namespace cordo_internal_cpo {

template <typename T, typename I, typename Rep, typename M>
constexpr auto customize(
    decltype(::cordo::mirror_unwrap), adl_tag,
    ::cordo::mirror_ref<::cordo_internal_mirror::mirror_option<T, I, Rep>, M>
        ref) CORDO_INTERNAL_ALIAS_(::cordo::mirror(*ref.rep()));

template <typename T, typename I, typename Rep>
constexpr auto customize(decltype(::cordo::mirror_traits_subscript_keys) algo,
                         adl_tag,
                         ::cordo_internal_mirror::mirror_option<T, I, Rep> t)
    CORDO_INTERNAL_ALIAS_(algo(::cordo::mirror.t(::cordo::tag_t<I>{})));

template <typename T, typename I, typename Rep, typename M, auto K>
constexpr decltype(auto) customize(
    decltype(::cordo::mirror_subscript_key) algo, adl_tag,
    ::cordo::mirror_ref<::cordo_internal_mirror::mirror_option<T, I, Rep>, M>
        ref,
    ::cordo::key_t<K> k) CORDO_INTERNAL_RETURN_(ref->unwrap()[k]);

template <typename T>
constexpr auto customize(decltype(::cordo::mirror_traits_ctor), adl_tag,
                         ::cordo::tag_t<std::unique_ptr<T>>) noexcept {
  return ::cordo_internal_mirror::mirror_option<std::unique_ptr<T>, T&>{};
}

template <typename T, typename I>
constexpr auto customize(
    decltype(::cordo::mirror_traits_of_const), adl_tag,
    ::cordo_internal_mirror::mirror_option<T, I, T&>) noexcept {
  return ::cordo_internal_mirror::mirror_option<const T, const I, const T&>{};
}

}  // namespace cordo_internal_cpo