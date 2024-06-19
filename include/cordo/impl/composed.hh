#pragma once

#include <type_traits>

namespace cordo {

template <typename O, typename I>
struct composed_t final {
  static_assert(std::is_same_v<typename O::value_t, typename I::tuple_t>);

  using tuple_t = typename O::tuple_t;
  using value_t = typename I::value_t;

  O outer_;
  I inner_;
};

inline constexpr struct {
  template <typename O, typename I>
  constexpr decltype(auto) operator()(O outer, I inner) const noexcept {
    return composed_t<O, I>{outer, inner};
  }

  template <typename O, typename I, typename... Is>
  constexpr decltype(auto) operator()(O outer, I inner,
                                      Is... is) const noexcept {
    return (*this)(composed_t<O, I>{outer, inner}, is...);
  }

} compose_{};

}  // namespace cordo