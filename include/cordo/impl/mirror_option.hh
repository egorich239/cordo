#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/mirror.hh"

namespace cordo_internal_mirror {

template <typename T, typename Rep = T&>
struct mirror_option final {
  using t = T;
  using rep = Rep;

  using name = ::cordo::null_t;
};

}  // namespace cordo_internal_mirror

namespace cordo_internal_cpo {

template <typename T, typename Rep>
constexpr auto customize(decltype(::cordo::mirror_unwrap), adl_tag,
                         ::cordo_internal_mirror::mirror_option<T, Rep>,
                         Rep&& opt) CORDO_INTERNAL_ALIAS_(*opt);

template <typename T, typename Rep>
constexpr auto customize(decltype(::cordo::mirror_traits_subscript_keys) algo,
                         adl_tag,
                         ::cordo_internal_mirror::mirror_option<T, Rep> t)
    CORDO_INTERNAL_ALIAS_(algo(::cordo::mirror.t(
        ::cordo::tag_t<decltype(::cordo::mirror_unwrap(
            t, std::declval<typename ::cordo_internal_mirror::mirror_option<
                   T, Rep>::rep&&>()))>{})));

template <typename T, typename Rep, auto K>
constexpr decltype(auto) customize(
    decltype(::cordo::mirror_subscript_key) algo, adl_tag,
    ::cordo_internal_mirror::mirror_option<T, Rep> t, Rep&& s,
    ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(
        /* TODO: this unwrap-wrap-unwrap is lame */ ::cordo::mirror(
            ::cordo::mirror_unwrap(t, s))[k]
            .v());

template <typename T>
constexpr auto customize(decltype(::cordo::mirror_traits_ctor), adl_tag,
                         ::cordo::tag_t<std::unique_ptr<T>&>) noexcept {
  return ::cordo_internal_mirror::mirror_option<std::unique_ptr<T>>{};
}

}  // namespace cordo_internal_cpo