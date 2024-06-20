#pragma once

#include <cstddef>
#include <string_view>

namespace cordo {
namespace cordo_literal_internal {
template <::std::size_t N>
struct literal_value_t final {
  template <char... C>
  constexpr literal_value_t() noexcept
    requires(sizeof...(C) == N)
      : value_{C...} {}

  constexpr literal_value_t(const char (&v)[N]) noexcept : value_{} {
    for (::std::size_t t = 0; t < N; ++t) value_[t] = v[t];
  }
  constexpr std::string_view operator()() const noexcept {
    return std::string_view{value_};
  }

  char value_[N];
};

template <char... L>
struct literal_t final {
  constexpr std::string_view operator()() const noexcept {
    return std::string_view{value_};
  }
  static constexpr const char value_[sizeof...(L)] = {L...};
};

struct parse_index_result_t final {
  ::std::size_t value;
  bool valid;
};

template <char... Cs>
constexpr parse_index_result_t parse_index() {
  using R = parse_index_result_t;
  constexpr size_t N = sizeof...(Cs);
  constexpr const char C[N] = {Cs...};

  if (N == 0) return {};
  if (N == 1) {
    return '0' <= C[0] && C[0] <= '9' ? R{.value = C[0] - '0', .valid = true}
                                      : R{};
  }

  constexpr int DEC = 1;
  constexpr int OCT = 2;
  constexpr int HEX = 3;

  int c = DEC;   // category
  size_t p = 0;  // pointer
  size_t d = 0;  // number of valid digits
  size_t r = 0;  // result

  if (C[0] == '0') c = OCT, ++p;
  if (p == 1 && (C[1] == 'x' || C[1] == 'X')) c = HEX, ++p;

  for (; p < N; ++p) {
    if (C[p] == '\'') continue;
    if (c == OCT && '0' <= C[p] && C[p] <= '7') {
      r *= 8;
      r += C[p] - '0';
      ++d;
      continue;
    }
    if (c == DEC && '0' <= C[p] && C[p] <= '9') {
      r *= 10;
      r += C[p] - '0';
      ++d;
      continue;
    }
    if (c == HEX && '0' <= C[p] && C[p] <= '9') {
      r *= 16;
      r += C[p] - '0';
      ++d;
      continue;
    }
    if (c == HEX && 'a' <= C[p] && C[p] <= 'f') {
      r *= 16;
      r += C[p] - 'a' + 10;
      ++d;
      continue;
    }
    if (c == HEX && 'A' <= C[p] && C[p] <= 'F') {
      r *= 16;
      r += C[p] - 'A' + 10;
      ++d;
      continue;
    }
    return {};
  }
  return d ? R{r, true} : R{};
}

template <::cordo::cordo_literal_internal::literal_value_t L>
constexpr decltype(auto) literal() noexcept {
  return []<size_t... I>(std::index_sequence<I...>) {
    return literal_t<L.value_[I]...>{};
  }(std::make_index_sequence<sizeof(decltype(L)::value_)>());
}

}  // namespace cordo_literal_internal

using cordo_literal_internal::literal_t;

namespace literals {
template <::cordo::cordo_literal_internal::literal_value_t L>
constexpr decltype(auto) operator""_t() noexcept {
  return ::cordo::cordo_literal_internal::literal<L>();
}
}  // namespace literals

}  // namespace cordo