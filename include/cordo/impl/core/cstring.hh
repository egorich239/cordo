#pragma once

#include <cstddef>
#include <string_view>
#include <type_traits>

#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cstring {
template <::std::size_t N>
struct cstring final {
  constexpr cstring() noexcept = default;
  constexpr cstring(const char (&v)[N + 1]) noexcept : value{} {
    for (::std::size_t t = 0; t <= N; ++t) value[t] = v[t];
  }
  constexpr ::std::size_t size() const noexcept { return N; }
  constexpr char operator[](::std::size_t idx) const noexcept {
    return value[idx];
  }
  constexpr std::string_view operator()() const noexcept {
    return std::string_view{value, value + N};
  }
  template <::std::size_t M>
  constexpr auto concat(cstring<M> other) const noexcept {
    cstring<N + M> result{};
    for (::std::size_t t = 0; t < N; ++t) result.value[t] = value[t];
    for (::std::size_t t = 0; t < M; ++t) result.value[t + N] = other.value[t];
    return result;
  }
  template <::std::size_t B, ::std::size_t E = N>
  constexpr auto substr(::cordo::value_t<B>,
                        ::cordo::value_t<E> = {}) const noexcept
    requires(N >= E && E >= B)
  {
    cstring<E - B> result{};
    for (::std::size_t t = B; t < E; ++t) result.value[t - B] = value[t];
    return result;
  }
  constexpr ::std::size_t find(char c) const noexcept {
    for (::std::size_t t = 0; t < N; ++t)
      if (value[t] == c) return t;
    return N;
  }
  constexpr ::std::size_t rfind(char c) const noexcept {
    for (::std::size_t t = N; t > 0;)
      if (value[--t] == c) return t + 1;
    return 0;
  }
  constexpr auto reverse() const noexcept {
    auto result = *this;
    for (::std::size_t t = 0; t < N / 2; ++t) {
      std::swap(result.value[t], result.value[N - 1 - t]);
    }
    return result;
  }

  constexpr auto operator<=>(const cstring&) const = default;
  constexpr bool operator==(const cstring&) const = default;
  constexpr bool operator!=(const cstring&) const = default;
  constexpr bool operator<=(const cstring&) const = default;
  constexpr bool operator>=(const cstring&) const = default;
  constexpr bool operator<(const cstring&) const = default;
  constexpr bool operator>(const cstring&) const = default;

  template <::std::size_t M>
  constexpr auto operator<(const cstring<M>&) const {
    return N < M;
  }
  template <::std::size_t M>
  constexpr auto operator<=(const cstring<M>&) const {
    return N < M;
  }
  template <::std::size_t M>
  constexpr auto operator>(const cstring<M>&) const {
    return N > M;
  }
  template <::std::size_t M>
  constexpr auto operator>=(const cstring<M>&) const {
    return N > M;
  }
  template <::std::size_t M>
  constexpr auto operator==(const cstring<M>&) const {
    return false;
  }
  template <::std::size_t M>
  constexpr auto operator!=(const cstring<M>&) const {
    return true;
  }

  char value[N + 1];
};
template <::std::size_t N>
cstring(const char (&)[N]) -> cstring<N - 1>;

static_assert(cstring("").reverse() == cstring(""));
static_assert(cstring("a").reverse() == cstring("a"));
static_assert(cstring("ab").reverse() == cstring("ba"));

inline constexpr struct {
  template <::std::size_t V>
  consteval auto operator()(::cordo::value_t<V>) const noexcept {
    if constexpr (V == 0) {
      return cstring("0");
    } else {
      constexpr auto digits = [] {
        auto v = V;
        auto res = 1;
        while (v /= 10) ++res;
        return res;
      }();
      constexpr auto result = [](auto dig) {
        constexpr auto digits = decltype(dig){}();
        char s[digits + 1] = {};
        auto v = V;
        for (std::size_t t = 0; t < digits; ++t) {
          const auto res = v % 10;
          s[digits - 1 - t] = char('0' + res);
          v /= 10;
        }
        return cstring(s);
      }(::cordo::value_t<digits>{});
      return result;
    }
  }
} to_cstring{};

static_assert(to_cstring(::cordo::value_t<(::std::size_t)0>{}) == cstring("0"));
static_assert(to_cstring(::cordo::value_t<(::std::size_t)9>{}) == cstring("9"));
static_assert(to_cstring(::cordo::value_t<(::std::size_t)12309>{}) ==
              cstring("12309"));

inline constexpr struct {
  struct R final {
    ::std::size_t value;
    bool valid;
  };

  template <char... Cs>
  constexpr R parse() const noexcept {
    constexpr size_t N = sizeof...(Cs);
    constexpr const char C[N] = {Cs...};

    if constexpr (N == 0) return {};
    if constexpr (N == 1) {
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

}  // namespace cordo_internal_cstring

namespace cordo {
using ::cordo_internal_cstring::cstring;
using ::cordo_internal_cstring::to_cstring;

namespace literals {
template <::cordo::cstring V>
constexpr auto operator""_cs() noexcept {
  return V;
}
}  // namespace literals
}  // namespace cordo