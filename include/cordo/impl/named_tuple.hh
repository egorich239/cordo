#pragma once

#include <type_traits>

#include "cordo/impl/literal.hh"
#include "cordo/impl/named.hh"

namespace cordo {

template <typename S, named_t... Fields>
struct named_tuple_t final {
  static_assert((std::is_same_v<typename decltype(Fields)::tuple_t, S> && ...));

  using tuple_t = S;
  template <char... C>
  constexpr decltype(auto) operator[](
      ::cordo::literal_t<C...> N) const noexcept {
    return this->resolve(N, Fields...);
  }

 private:
  template <typename N, typename F0, typename... Fs>
  constexpr decltype(auto) resolve(N n, F0 h, Fs... t) const noexcept {
    return this->resolve2(n, h.name(), h.accessor_, t...);
  }

  template <typename N, typename A, typename... Fs>
  constexpr decltype(auto) resolve2(N, N, A a, Fs...) const noexcept {
    return a;
  }

  template <typename N, typename H, typename A, typename... Fs>
  constexpr decltype(auto) resolve2(N n, H, A, Fs... t) const noexcept {
    return this->resolve(n, t...);
  }
};

}  // namespace cordo