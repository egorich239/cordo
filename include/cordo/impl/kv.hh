#pragma once

#include <type_traits>
#include <utility>

#include "cordo/impl/literal.hh"
#include "cordo/impl/value.hh"

namespace cordo {
namespace cordo_kv_internal {
template <auto K>
struct key_t final {
  template <typename V>
  struct kv_t final {
    constexpr decltype(auto) key() const noexcept { return K; }
    constexpr const V& value() const noexcept { return value_; }
    V value_;
  };

  constexpr decltype(auto) operator()() const noexcept { return K; }

  template <typename V>
  constexpr decltype(auto) operator=(V&& v) {
    return kv_t<V>{std::forward<V>(v)};
  }
};

}  // namespace cordo_kv_internal

template <auto K>
using key_t = typename ::cordo::cordo_kv_internal::key_t<K>;

namespace literals {

template <char... C>
constexpr decltype(auto) operator""_key() noexcept
  requires(::cordo::cordo_literal_internal::parse_index<C...>().valid)
{
  constexpr ::std::size_t VALUE =
      ::cordo::cordo_literal_internal::parse_index<C...>().value;
  return ::cordo::key_t<::cordo::value_t<VALUE>{}>{};
}

template <::cordo::cordo_literal_internal::literal_value_t L>
constexpr auto operator""_key() noexcept {
  return ::cordo::key_t<::cordo::cordo_literal_internal::literal<L>()>{};
}

}  // namespace literals
}  // namespace cordo