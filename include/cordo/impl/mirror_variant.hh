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

namespace cordo_internal_mirror {

struct mirror_variant_size_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;
};
struct mirror_variant_index_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;
};
struct mirror_variant_get_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;
};

inline constexpr ::cordo::algo<mirror_variant_size_t> mirror_variant_size;
inline constexpr ::cordo::algo<mirror_variant_index_t> mirror_variant_index;
inline constexpr ::cordo::algo<mirror_variant_get_t> mirror_variant_get;

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
struct mirror_variant final {
  using t = T;
  using rep = T&;

  using name = ::cordo::null_t;
  using subscript_map =
      decltype(mirror_variant_make_options(::cordo::tag_t<T>{}, Options{}));
  using subscript_keys =
      decltype(::cordo_internal_mirror::mirror_struct_eval_keys(
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
  /* TODO: something is severely broken with mirror_api assignment operator... */
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

}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirror_variant_get;
using ::cordo_internal_mirror::mirror_variant_index;
using ::cordo_internal_mirror::mirror_variant_size;
}  // namespace cordo

namespace cordo_internal_cpo {

// mirror_variant
template <typename T, typename Options>
constexpr auto customize(
    decltype(::cordo::mirror_traits_of_const), adl_tag,
    ::cordo_internal_mirror::mirror_variant<T, Options>) noexcept {
  return ::cordo_internal_mirror::mirror_variant<const T, Options>{};
}

template <typename T, typename Options, typename EH, auto K>
constexpr decltype(auto) customize(
    decltype(::cordo::mirror_subscript_key), adl_tag,
    ::cordo::mirror_core<cordo_internal_mirror::mirror_variant<T, Options>, EH>&
        core,
    ::cordo::key_t<K> k) noexcept {
  using traits = decltype(core.traits());
  constexpr size_t Idx =
      ::cordo::kv_lookup(typename traits::subscript_map{}, decltype(k){});
  using F = decltype(cordo_internal_mirror::mirror_variant_get(
      core.value, ::cordo::value_t<Idx>{}));
  return ::cordo::mirror.core(
      ::cordo_internal_mirror::mirror_variant_option<traits, F, Idx>{
          core.value},
      EH{});
}

template <typename T, typename Options, typename EH, auto K>
constexpr decltype(auto) customize(
    decltype(::cordo::mirror_subscript_key), adl_tag,
    const ::cordo::mirror_core<
        cordo_internal_mirror::mirror_variant<T, Options>, EH>& core,
    ::cordo::key_t<K> k) noexcept {
  using traits = decltype(core.traits());
  constexpr size_t Idx =
      ::cordo::kv_lookup(typename traits::subscript_map{}, decltype(k){});
  using F = decltype(cordo_internal_mirror::mirror_variant_get(
      core.value, ::cordo::value_t<Idx>{}));
  return ::cordo::mirror.core(
      ::cordo_internal_mirror::mirror_variant_option<traits, F, Idx>{
          core.value},
      EH{});
}

// mirror_variant_option
template <typename Traits, typename F, size_t I>
constexpr auto customize(
    decltype(::cordo::mirror_traits_ctor), adl_tag,
    ::cordo::tag_t<::cordo_internal_mirror::mirror_variant_option<
        Traits, F, I>&&>) noexcept {
  using Opt = ::cordo_internal_mirror::mirror_variant_option<Traits, F, I>;
  return ::cordo_internal_mirror::mirror_option<Opt, F, Opt>{};
}

template <
    typename..., typename Traits, typename F, size_t I,
    typename Opt = ::cordo_internal_mirror::mirror_variant_option<Traits, F, I>>
constexpr auto customize(
    decltype(::cordo::mirror_traits_of_const), adl_tag,
    ::cordo_internal_mirror::mirror_option<Opt, F, Opt>) noexcept {
  using OptC =
      ::cordo_internal_mirror::mirror_variant_option<Traits, const F, I>;
  return ::cordo_internal_mirror::mirror_option<OptC, const F, OptC>{};
}

template <typename Traits, typename F, size_t I>
constexpr auto customize(
    decltype(::cordo::mirror_traits_subscript_keys) algo, adl_tag,
    ::cordo_internal_mirror::mirror_option<
        ::cordo_internal_mirror::mirror_variant_option<Traits, F, I>, F,
        ::cordo_internal_mirror::mirror_variant_option<Traits, F,
                                                       I>>) noexcept {
  return algo(typename decltype(::cordo::mirror(std::declval<F>()))::traits{});
}

// std::variant
template <typename... T>
constexpr auto customize(decltype(::cordo::mirror_variant_size), adl_tag,
                         ::cordo::tag_t<std::variant<T...>>) noexcept {
  return sizeof...(T);
}

template <typename... T, size_t I>
constexpr decltype(auto) customize(decltype(::cordo::mirror_variant_get),
                                   adl_tag, std::variant<T...>& v,
                                   ::cordo::value_t<I>) noexcept {
  return std::get<I>(v);
}

template <typename... T, size_t I>
constexpr decltype(auto) customize(decltype(::cordo::mirror_variant_get),
                                   adl_tag, const std::variant<T...>& v,
                                   ::cordo::value_t<I>) noexcept {
  return std::get<I>(v);
}

template <typename... T>
constexpr auto customize(decltype(::cordo::mirror_variant_index), adl_tag,
                         std::variant<T...>& v) noexcept {
  return v.index();
}

}  // namespace cordo_internal_cpo