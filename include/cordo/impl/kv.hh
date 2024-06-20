#pragma once

#include <type_traits>
#include <utility>

#include "cordo/impl/literal.hh"

namespace cordo_internal_kv {
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

template <auto V>
struct value_t final {
  constexpr decltype(auto) operator()() const noexcept { return V; }
};
}  // namespace cordo_internal_kv

namespace cordo {

using ::cordo_internal_kv::key_t;
using ::cordo_internal_kv::value_t;
template <auto K, typename V>
using kv_t = typename ::cordo_internal_kv::key_t<K>::template kv_t<V>;

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