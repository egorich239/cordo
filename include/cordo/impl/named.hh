#pragma once

#include "cordo/impl/literal.hh"

namespace cordo {

template <::cordo::literal_t N, typename A>
struct named_t final {
  using tuple_t = typename A::tuple_t;
  using value_t = typename A::value_t;
  using name_t = decltype(N);

  constexpr name_t name() const noexcept { return name_t{}; }
  A accessor_;
};

inline constexpr struct {
  template <typename L, typename A>
  constexpr decltype(auto) operator()(L, A accessor) const noexcept {
    return named_t<L{}, A>{accessor};
  }
} named_{};

}  // namespace cordo