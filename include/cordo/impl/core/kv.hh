#pragma once

#include <type_traits>
#include <utility>

#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_kv {
template <auto K>
struct key_t;

template <key_t K, typename V>
struct kv_t final {
  using value_t = V;
  constexpr auto key() const noexcept { return K; }
  constexpr const V& value() const noexcept { return value_; }

  template <typename V2>
  constexpr auto operator||(V2 v2) const noexcept {
    auto res = ::cordo::li_push(v2, ::cordo::make_li(value_));
    return kv_t<K, decltype(res)>{res};
  }

  V value_;
};

template <auto K>
struct key_t final {
  constexpr auto operator()() const noexcept { return K; }

  template <typename V>
  constexpr decltype(auto) operator=(V v) const {
    return kv_t<key_t<K>{}, V>{v};
  }

  template <typename V>
  constexpr decltype(auto) operator<=(V v) const {
    return kv_t<key_t<K>{}, V>{v};
  }
};

struct kv_lookup_t final {
  template <auto K>
  constexpr auto operator()(::cordo::values_t<>, key_t<K>) const noexcept {
    CORDO_INTERNAL_DIAG_("key not found among the value pairs");
  }

  template <auto V, auto... Vs, auto K>
  constexpr auto operator()(::cordo::values_t<V, Vs...>,
                            key_t<K> k) const noexcept {
    return this->resolve(k, V.key(), V.value(), ::cordo::values_t<Vs...>{});
  }

 private:
  template <typename K, typename V, typename Vs>
  constexpr auto resolve(K, K, V v, Vs) const noexcept {
    return v;
  }

  template <typename K, typename K2, typename V, typename Vs>
  constexpr auto resolve(K k, K2, V, Vs vs) const noexcept {
    return (*this)(vs, k);
  }
};

inline constexpr struct {
  template <auto V>
  constexpr auto operator()(::cordo::overload_prio_t<3>,
                            ::cordo::value_t<key_t<V>{}>) const noexcept {
    return key_t<V>{};
  }
  template <::cordo::null_t V>
  constexpr auto operator()(::cordo::overload_prio_t<2>,
                            ::cordo::value_t<V>) const noexcept {
    return ::cordo::null_t{};
  }
  template <auto V>
  constexpr auto operator()(::cordo::overload_prio_t<1>,
                            ::cordo::value_t<V>) const noexcept {
    return key_t<V>{};
  }
} make_key_impl{};

template <auto V>
using make_key = decltype(make_key_impl(::cordo::overload_prio_t<3>{},
                                        ::cordo::value_t<V>{}));

}  // namespace cordo_internal_kv

namespace cordo {
using ::cordo_internal_kv::key_t;
using ::cordo_internal_kv::kv_t;
using ::cordo_internal_kv::make_key;
inline constexpr ::cordo_internal_kv::kv_lookup_t kv_lookup{};

namespace literals {

template <char... C>
constexpr decltype(auto) operator""_key() noexcept
  requires(::cordo_internal_cstring::as_index_t.parse<C...>().valid)
{
  constexpr auto VALUE =
      ::cordo_internal_cstring::as_index_t.parse<C...>().value;
  return ::cordo::make_key<VALUE>{};
}

template <::cordo::cstring L>
constexpr auto operator""_key() noexcept {
  return ::cordo::make_key<L>{};
}

}  // namespace literals
}  // namespace cordo