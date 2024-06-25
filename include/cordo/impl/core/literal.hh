#pragma once

#include <cstddef>
#include <string_view>

namespace cordo_internal_literal {
template <::std::size_t N>
struct string_v final {
  template <char... C>
  constexpr string_v() noexcept
    requires(sizeof...(C) == N)
      : value_{C...} {}

  constexpr string_v(const char (&v)[N]) noexcept : value_{} {
    for (::std::size_t t = 0; t < N; ++t) value_[t] = v[t];
  }
  constexpr std::string_view operator()() const noexcept {
    return std::string_view{value_};
  }

  char value_[N];
};

template <char... L>
struct string_t final {
  constexpr std::string_view operator()() const noexcept {
    return std::string_view{value_};
  }
  static constexpr const char value_[sizeof...(L)] = {L...};
};

inline constexpr struct {
  struct R final {
    ::std::size_t value;
    bool valid;
  };

  template <char... Cs>
  constexpr R parse() const noexcept {
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
} as_index_t{};

inline constexpr struct {
  template <string_v L>
  constexpr decltype(auto) parse() const noexcept {
    return []<size_t... I>(std::index_sequence<I...>) {
      return string_t<L.value_[I]...>{};
    }(std::make_index_sequence<sizeof(decltype(L)::value_)>());
  }
} as_string_t{};

}  // namespace cordo_internal_literal
