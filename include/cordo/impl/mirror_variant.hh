#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/mirror.hh"
#include "cordo/impl/mirror_option.hh"
#include "cordo/impl/mirror_struct.hh"

namespace cordo {
namespace cordo_internal_mirror {

struct mirror_variant_size_core_t final {};
struct mirror_variant_index_core_t final {};
struct mirror_variant_get_core_t final {};

inline constexpr ::cordo::algo_t<cpo_v<mirror_variant_size_core_t{}>>
    mirror_variant_size;
inline constexpr ::cordo::algo_t<cpo_v<mirror_variant_index_core_t{}>>
    mirror_variant_index;
inline constexpr ::cordo::algo_t<cpo_v<mirror_variant_get_core_t{}>>
    mirror_variant_get;

inline constexpr struct {
 private:
  template <auto... K, size_t... Idx>
  constexpr auto eval(::cordo::types_t<::cordo::key_t<K>...>,
                      std::integer_sequence<size_t, Idx...>) const noexcept
    requires(sizeof...(K) == sizeof...(Idx))
  {
    return ::cordo::values_t<(::cordo::key_t<K>{} <= Idx)...>{};
  }
  template <auto... K, size_t... Idx>
  constexpr auto eval(::cordo::types_t<::cordo::key_t<K>...>,
                      std::integer_sequence<size_t, Idx...>) const noexcept
    requires(sizeof...(K) != sizeof...(Idx))
  {
    CORDO_INTERNAL_DIAG_(
        "number of Options must match the number of variant branches");
  }

 public:
  template <typename V, auto... Ks>
  constexpr auto operator()(::cordo::tag_t<V>,
                            ::cordo::values_t<Ks...>) const noexcept {
    return this->eval(::cordo::types_t<std::remove_const_t<decltype(Ks)>...>{},
                      std::make_index_sequence<mirror_variant_size(
                          ::cordo::tag_t<std::remove_cv_t<V>>{})>{});
  }

} mirror_variant_make_options{};

template <typename T, typename Options>
struct mirror_variant_traits final {
  using t = T;
  using rep = T&;

  using name = ::cordo::null_t;
  using subscript_map =
      decltype(mirror_variant_make_options(::cordo::tag_t<T>{}, Options{}));
  using subscript_keys =
      decltype(cordo::cordo_internal_mirror::mirror_struct_eval_keys(
          subscript_map{}));
};

template <typename Traits, typename F, size_t I>
struct mirror_variant_option final {
  using rep = typename Traits::rep;
  rep variant_;

  constexpr explicit operator bool() const noexcept {
    return this->variant_.index() == I;
  }
  constexpr decltype(auto) operator*() { return std::get<I>(this->variant_); }
  constexpr decltype(auto) operator*() const {
    return std::get<I>(this->variant_);
  }
  /* TODO: something is severely broken with mirror_api assignment operator...
   */
  template <
      typename..., typename U, typename T2 = Traits,
      typename = std::enable_if_t<!std::is_const_v<typename T2::t>>,
      typename R = decltype(std::declval<typename T2::t&>() =
                                std::declval<std::remove_reference_t<F>>())>
  constexpr auto operator=(U&& v) noexcept(
      noexcept(this->variant_ = std::remove_reference_t<F>{(U&&)v})) -> R
    requires(
        /* TODO: F should inherit constness */
        !std::is_const_v<typename Traits::t> &&
        std::is_constructible_v<std::remove_reference_t<F>, U &&>)
  {
    return this->variant_ = std::remove_reference_t<F>{(U&&)v};
  }
};

// mirror_variant
template <typename T, typename Options>
constexpr auto customize(hook_t<mirror_traits_of_const>,
                         mirror_variant_traits<T, Options>) noexcept {
  return mirror_variant_traits<const T, Options>{};
}

template <typename T, typename Options, auto K>
constexpr decltype(auto) customize(hook_t<mirror_subscript_key>,
                                   mirror_variant_traits<T, Options>,
                                   auto&& core, ::cordo::key_t<K> k) noexcept {
  using traits = decltype(core.traits());
  constexpr size_t Idx =
      ::cordo::kv_lookup(typename traits::subscript_map{}, decltype(k){});
  using F = decltype(mirror_variant_get(core.value, ::cordo::value_t<Idx>{}));
  return core |
         cordo::piped(make_mirror_impl, mirror_variant_option<traits, F, Idx>{
                                            ((decltype(core)&&)core).value});
}

// mirror_variant_option
template <typename Traits, typename F, size_t I>
constexpr auto customize(
    hook_t<mirror_traits_ctor>,
    ::cordo::tag_t<mirror_variant_option<Traits, F, I>&&>) noexcept {
  using Opt = mirror_variant_option<Traits, F, I>;
  return mirror_option_traits<Opt, F, Opt>{};
}

template <typename..., typename Traits, typename F, size_t I,
          typename Opt = mirror_variant_option<Traits, F, I>>
constexpr auto customize(hook_t<mirror_traits_of_const>,
                         mirror_option_traits<Opt, F, Opt>) noexcept {
  using OptC = mirror_variant_option<Traits, const F, I>;
  return mirror_option_traits<OptC, const F, OptC>{};
}

template <typename Traits, typename F, size_t I>
constexpr auto customize(
    hook_t<mirror_traits_subscript_keys>,
    mirror_option_traits<mirror_variant_option<Traits, F, I>, F,
                         mirror_variant_option<Traits, F, I>>) noexcept {
  return mirror_traits_subscript_keys(
      typename decltype(::cordo::mirror(std::declval<F>()))::traits{});
}

// std::variant
template <typename... T>
constexpr auto customize(hook_t<mirror_variant_size>,
                         ::cordo::tag_t<std::variant<T...>>) noexcept {
  return sizeof...(T);
}

template <typename... T, size_t I>
constexpr decltype(auto) customize(hook_t<mirror_variant_get>,
                                   std::variant<T...>& v,
                                   ::cordo::value_t<I>) noexcept {
  return std::get<I>(v);
}

template <typename... T, size_t I>
constexpr decltype(auto) customize(hook_t<mirror_variant_get>,
                                   const std::variant<T...>& v,
                                   ::cordo::value_t<I>) noexcept {
  return std::get<I>(v);
}

template <typename... T>
constexpr auto customize(hook_t<mirror_variant_index>,
                         std::variant<T...>& v) noexcept {
  return v.index();
}

}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_variant_get;
using cordo_internal_mirror::mirror_variant_index;
using cordo_internal_mirror::mirror_variant_size;
using cordo_internal_mirror::mirror_variant_traits;

template <typename M>
concept mirror_variant = requires(typename std::remove_cvref_t<M>::traits t) {
  requires is_mirror<M>;
  { mirror_variant_traits{t} } -> std::same_as<decltype(t)>;
};

}  // namespace cordo