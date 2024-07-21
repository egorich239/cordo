#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>

#include "cordo/impl/core/cpo.hh"
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
                      std::integer_sequence<size_t, Idx...>) const noexcept {
    return ::cordo::values_t<(::cordo::key_t<K>{} <=
                              ::cordo::value_t<Idx>{})...>{};
  }

 public:
  template <typename V, auto... Ks>
  constexpr auto operator()(::cordo::tag_t<V>, ::cordo::value_t<Ks...>) const
      CORDO_INTERNAL_ALIAS_(
          this->eval(::cordo::types_t<decltype(Ks)...>{},
                     std::make_index_sequence<
                         mirror_variant_size(::cordo::tag_t<V>{})>{}));
} mirror_variant_make_options{};

template <typename T, typename Options>
struct mirror_variant final {
  using t = T;
  using rep = T&;

  using name = ::cordo::null_t;
  using subscript_map = decltype(mirror_variant_make_options(Options{}));
  using subscript_keys =
      decltype(::cordo_internal_mirror::mirror_struct_eval_keys(
          subscript_map{}));
};

template <typename Traits, size_t I>
struct mirror_variant_option final {
  using rep = typename Traits::rep;
  rep variant_;

  constexpr decltype(auto) operator*() { return std::get<I>(this->variant_); }
  constexpr decltype(auto) operator*() const {
    return std::get<I>(this->variant_);
  }
};

}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirror_variant_get;
using ::cordo_internal_mirror::mirror_variant_index;
using ::cordo_internal_mirror::mirror_variant_size;
}  // namespace cordo

namespace cordo_internal_cpo {

template <typename... T>
constexpr auto customize(decltype(::cordo::mirror_variant_size), adl_tag,
                         ::cordo::tag_t<std::variant<T...>>) noexcept {
  return sizeof...(T);
}

template <typename... T, size_t I>
constexpr auto customize(decltype(::cordo::mirror_variant_get), adl_tag,
                         std::variant<T...>& v, ::cordo::value_t<I>) noexcept {
  return std::get<I>(v);
}

template <typename... T>
constexpr auto customize(decltype(::cordo::mirror_variant_index), adl_tag,
                         std::variant<T...>& v) noexcept {
  return v.index();
}

template <typename Traits, size_t I>
constexpr auto customize(
    decltype(::cordo::mirror_traits_ctor), adl_tag,
    ::cordo_internal_mirror::mirror_variant_option<Traits, I> opt) noexcept {
  return ::cordo_internal_mirror::mirror_option<decltype(opt), decltype(opt)>{};
}



}  // namespace cordo_internal_cpo