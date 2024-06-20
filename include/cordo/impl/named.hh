#pragma once

#include "cordo/impl/accessor.hh"
#include "cordo/impl/kv.hh"

namespace cordo_internal_named {
template <::cordo::key_t N, ::cordo::accessor A>
struct named_t final {
  using tuple_t = typename A::tuple_t;
  using value_t = typename A::value_t;

  constexpr decltype(auto) name() const noexcept { return N(); }
  A accessor_;
};
}  // namespace cordo_internal_named

namespace cordo {
inline constexpr struct {
  template <auto K, accessor A>
  constexpr decltype(auto) operator()(key_t<K>, A accessor) const noexcept {
    return ::cordo_internal_named::named_t<key_t<K>{}, A>{accessor};
  }
} named{};
}  // namespace cordo