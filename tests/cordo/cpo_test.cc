#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <type_traits>

#include "cordo/cordo.hh"

namespace n1 {
struct adl_tag final {};
}  // namespace n1
namespace n2 {
struct adl_tag final {};
}  // namespace n2

namespace n3 {
constexpr int sum(int x, int y) { return x + y; }

inline constexpr cordo::cpo_t<sum> sum_algo{};
inline constexpr cordo::cpo_t<sum, n1::adl_tag> sum_algo_n1{};
inline constexpr cordo::cpo_t<sum, n2::adl_tag, n1::adl_tag> sum_algo_n2n1{};
}  // namespace n3

namespace n4 {
struct Foo {
  int x;
};

Foo customize(cordo::hook_t<n3::sum_algo>, Foo a, Foo b) {
  return Foo{a.x + b.x};
}
}  // namespace n4

namespace n1 {
std::string customize(cordo::hook_t<n3::sum_algo_n1, adl_tag>, std::string x,
                      std::string y) {
  return x + y;
}
}  // namespace n1

namespace n2 {
std::string customize(cordo::hook_t<n3::sum_algo_n2n1, adl_tag>, std::string x,
                      std::string y) {
  return x + " " + y;
}
}  // namespace n2

namespace {

TEST(Sum, WithoutTags) {
  EXPECT_THAT(cordo::cpo_invoke(n3::sum_algo, 2, 3), 5);
  auto foo = cordo::cpo_invoke(n3::sum_algo, n4::Foo{5}, n4::Foo{-12});
  static_assert(std::is_same_v<decltype(foo), n4::Foo>);
  EXPECT_THAT(foo.x, -7);
}

TEST(Sum, WithN1) {
  EXPECT_THAT(cordo::cpo_invoke(n3::sum_algo_n1, 2, 3), 5);
  auto foo = cordo::cpo_invoke(n3::sum_algo_n1, n4::Foo{5}, n4::Foo{-12});
  static_assert(std::is_same_v<decltype(foo), n4::Foo>);
  EXPECT_THAT(foo.x, -7);
  std::string res = cordo::cpo_invoke(n3::sum_algo_n1, "lorem", "ipsum");
  EXPECT_THAT(res, testing::StrEq("loremipsum"));
}

TEST(Sum, WithN2N1) {
  EXPECT_THAT(cordo::cpo_invoke(n3::sum_algo_n2n1, 2, 3), 5);
  auto foo = cordo::cpo_invoke(n3::sum_algo_n2n1, n4::Foo{5}, n4::Foo{-12});
  static_assert(std::is_same_v<decltype(foo), n4::Foo>);
  EXPECT_THAT(foo.x, -7);
  std::string res = cordo::cpo_invoke(n3::sum_algo_n2n1, "lorem", 2 ) ;//"ipsum");
  EXPECT_THAT(res, testing::StrEq("lorem ipsum"));
}

}  // namespace