#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <type_traits>

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/hook.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/core/pipe.hh"
#include "cordo/impl/mirror.hh"

namespace cordo {
namespace cordo_internal_mirror {

template <typename T, typename I, typename Rep = T&>
struct mirror_option_traits final {
  using t = T;
  using rep = Rep;
  using inner = I;

  using name = ::cordo::null_t;
};

template <typename T, typename I, typename Rep>
constexpr auto customize(hook_t<mirror_has_value>,
                         mirror_option_traits<T, I, Rep>, auto&& core)
    CORDO_INTERNAL_RETURN_(static_cast<bool>(core.value));

template <typename T, typename I, typename Rep>
constexpr decltype(auto) customize(hook_t<mirror_unwrap>,
                                   mirror_option_traits<T, I, Rep> t,
                                   auto&& core)
    CORDO_INTERNAL_RETURN_(
        mirror_has_value(t, core)
            ? (core |
               cordo::piped(make_mirror_impl, *((decltype(core)&&)core).value) |
               cordo::piped(make_mirror_result))
            : (core | cordo::piped(make_mirror_error,
                                   ::cordo::mirror_error::INVALID_UNWRAP)));

template <typename T, typename I, typename Rep>
constexpr auto customize(hook_t<mirror_traits_subscript_keys>,
                         mirror_option_traits<T, I, Rep> t)
    CORDO_INTERNAL_RETURN_(
        mirror_traits_subscript_keys(mirror_traits_ctor(::cordo::tag_t<I>{})));

template <typename T, typename I, typename Rep, auto K>
constexpr decltype(auto) customize(hook_t<mirror_subscript_key>,
                                   mirror_option_traits<T, I, Rep>, auto&& core,
                                   ::cordo::key_t<K> k)
    CORDO_INTERNAL_RETURN_(mirror_impl_apply((decltype(core)&&)core,
                                             mirror_unwrap) |
                           cordo::piped(mirror_impl_apply, mirror_subscript_key,
                                        k));

template <typename T>
constexpr auto customize(hook_t<mirror_traits_ctor>,
                         ::cordo::tag_t<std::unique_ptr<T>>) noexcept {
  return mirror_option_traits<std::unique_ptr<T>, T&>{};
}

template <typename T>
constexpr auto customize(hook_t<mirror_traits_ctor>,
                         ::cordo::tag_t<std::optional<T>>) noexcept {
  return mirror_option_traits<std::optional<T>, T&>{};
}

template <typename T, typename I>
constexpr auto customize(hook_t<mirror_traits_of_const>,
                         mirror_option_traits<T, I, T&>) noexcept {
  return mirror_option_traits<const T, const I, const T&>{};
}

}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_option_traits;

template <typename M>
concept mirror_option = requires(typename std::remove_cvref_t<M>::traits t) {
  requires is_mirror<M>;
  { mirror_option_traits{t} } -> std::same_as<decltype(t)>;
};

}  // namespace cordo