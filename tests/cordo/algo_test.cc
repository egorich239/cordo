
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include "cordo/cordo.hh"

namespace {
namespace n1 {

struct adl_tag {};

}  // namespace n1

struct algo1_traits {
  constexpr auto operator()(std::string_view) const { return 1; }
};
inline constexpr ::cordo::algo_t<cordo::cpo_v<algo1_traits{}>> algo1{};

struct Foo {};
constexpr auto customize(cordo::hook_t<algo1>, Foo) { return 2; }

struct algo3_traits {
  template <typename... T>
  constexpr auto operator()(T... v) const noexcept {
    return (0 + ... + v);
  }
};
inline constexpr ::cordo::algo_t<cordo::cpo_v<algo3_traits{}>> algo3{};

struct Bar {
  constexpr int bar(int x) const { return x + 4; }
};

}  // namespace

namespace {

consteval void algo_test() {
  static_assert(algo1("foo") == 1);
  static_assert(algo1(Foo{}) == 2);
  static_assert(algo3(1, 2, 3) == 6);
}

}  // namespace