
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string_view>

#include "cordo/cordo.hh"

namespace {
namespace n1 {

struct adl_tag {};

}  // namespace n1

struct algo1_traits {
  constexpr auto operator()(const cordo::algo<algo1_traits>&,
                            std::string_view) const {
    return 1;
  }
};
inline constexpr ::cordo::algo<algo1_traits> algo1{};

struct Foo {};
constexpr auto customize(decltype(algo1), Foo) { return 2; }

struct algo2_traits {
  using adl_tag = n1::adl_tag;
};
inline constexpr ::cordo::algo<algo2_traits> algo2{};

namespace n1 {
constexpr auto customize(decltype(algo2), adl_tag, Foo) { return 3; }
}  // namespace n1
}  // namespace

namespace {

consteval void algo_test() {
  static_assert(algo1("foo") == 1);
  static_assert(algo1(Foo{}) == 2);
  static_assert(algo2(Foo{}) == 3);
}

}  // namespace