#pragma once

#include <type_traits>
#include <utility>

#include "cordo/impl/literal.hh"

namespace cordo_internal_kv {
template <auto K>
struct key_t;

template <key_t K, typename V>
struct kv_t final {
  using value_t = V;
  constexpr auto key() const noexcept { return K; }
  constexpr const V& value() const noexcept { return value_; }
  V value_;
};

template <auto K>
struct key_t final {
  constexpr auto operator()() const noexcept { return K; }

  template <typename V>
  constexpr decltype(auto) operator=(V v) const {
    return kv_t<key_t<K>{}, V>{v};
  }
};

template <auto V>
struct value_t final {
  constexpr auto operator()() const noexcept { return V; }
};
}  // namespace cordo_internal_kv

namespace cordo {

using ::cordo_internal_kv::key_t;
using ::cordo_internal_kv::value_t;
using ::cordo_internal_kv::kv_t;

namespace literals {

template <char... C>
constexpr decltype(auto) operator""_key() noexcept
  requires(::cordo_internal_literal::as_index_t.parse<C...>().valid)
{
  constexpr ::std::size_t VALUE =
      ::cordo_internal_literal::as_index_t.parse<C...>().value;
  return key_t<value_t<VALUE>{}>{};
}

template <::cordo_internal_literal::string_v L>
constexpr auto operator""_key() noexcept {
  return ::cordo::key_t<::cordo_internal_literal::as_string_t.parse<L>()>{};
}

}  // namespace literals
}  // namespace cordo