#pragma once

#include <cstdint>
#include <string_view>

namespace cordo {
namespace cordo_literal_internal {
template <::std::size_t N>
struct value_t final {
  constexpr value_t(const char (&v)[N]) noexcept : value_{} {
    for (::std::size_t t = 0; t < N; ++t) value_[t] = v[t];
  }
  constexpr std::string_view operator()() const noexcept {
    return std::string_view{value_};
  }

  char value_[N];
};
}  // namespace cordo_literal_internal

template <char... L>
struct literal_t final {
  constexpr std::string_view operator()() const noexcept {
    return std::string_view{value_};
  }
  static constexpr const char value_[sizeof...(L)] = {L...};
};

namespace literals {
template <::cordo::cordo_literal_internal::value_t L>
constexpr auto operator""_t() noexcept {
  return []<size_t... I>(std::index_sequence<I...>) {
    return literal_t<L.value_[I]...>{};
  }(std::make_index_sequence<sizeof(decltype(L)::value_)>());
}
}  // namespace literals

}  // namespace cordo