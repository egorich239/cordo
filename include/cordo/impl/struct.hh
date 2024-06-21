#pragma once

#include <type_traits>

#include "cordo/impl/kv.hh"
#include "cordo/impl/literal.hh"
#include "cordo/impl/meta.hh"
#include "cordo/impl/named.hh"

namespace cordo_internal_struct {

template <::cordo::key_t Name, typename S, ::cordo::kv_t... Fields>
struct struct_ final {
  static_assert((
      ::cordo::accessor<std::remove_cvref_t<decltype(Fields.value())>> && ...));

  using tuple_t = S;
  using fields_t = ::cordo::values_t<Fields...>;

  constexpr auto key() const noexcept { return Name; }
  constexpr auto fields() const noexcept { return fields_t{}; }

  template <auto K>
  constexpr auto operator[](::cordo::key_t<K> k) const noexcept {
    return this->resolve(k, Fields...);
  }

 private:
  template <typename K, typename F0, typename... Fs>
  constexpr auto resolve(K k, F0 h, Fs... t) const noexcept {
    return this->resolve2(k, h.key(), h.value(), t...);
  }

  template <typename K, typename A, typename... Fs>
  constexpr auto resolve2(K, K, A a, Fs...) const noexcept {
    return a;
  }

  template <typename K, typename K2, typename A, typename... Fs>
  constexpr decltype(auto) resolve2(K k, K2, A, Fs... t) const noexcept {
    return this->resolve(k, t...);
  }
};
}  // namespace cordo_internal_struct

namespace cordo {
using ::cordo_internal_struct::struct_;
}  // namespace cordo