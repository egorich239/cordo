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

template <typename T>
struct mirror_option final {
  using t = T;
  using rep = T&;

  using name = ::cordo::null_t;
};

}  // namespace cordo_internal_mirror

namespace cordo_internal_cpo {

template <typename T>
constexpr auto customize(decltype(::cordo::mirror_unwrap), adl_tag,
                         ::cordo_internal_mirror::mirror_option<T>, T& opt)
    CORDO_INTERNAL_ALIAS_(*opt);

template <typename T>
constexpr auto customize(decltype(::cordo::mirror_traits_subscript_keys) algo,
                         adl_tag, ::cordo_internal_mirror::mirror_option<T> t)
    CORDO_INTERNAL_ALIAS_(algo(::cordo::mirror.traits(::cordo::mirror_unwrap(
        t, std::declval<
               typename ::cordo_internal_mirror::mirror_option<T>::rep>()))));

template <typename T, auto K>
constexpr auto customize(decltype(::cordo::mirror_subscript_key) algo, adl_tag,
                         ::cordo_internal_mirror::mirror_option<T> t, T& s,
                         ::cordo::key_t<K> k)
    CORDO_INTERNAL_ALIAS_(
        /* TODO: this unwrap-wrap-unwrap is lame */ ::cordo::mirror(
            ::cordo::mirror_unwrap(t, s))[k]
            .v());

template <typename T>
constexpr auto customize(decltype(::cordo::mirror_traits_ctor), adl_tag,
                         ::cordo::tag_t<std::unique_ptr<T>>) noexcept {
  return ::cordo_internal_mirror::mirror_option<std::unique_ptr<T>>{};
}

}  // namespace cordo_internal_cpo