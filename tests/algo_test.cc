
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

struct algo3_traits {
  template <typename... T>
  constexpr auto operator()(const cordo::algo<algo3_traits>&,
                            T... v) const noexcept {
    return (0 + ... + v);
  }
};
inline constexpr ::cordo::algo<algo3_traits> algo3{};

template <template <typename, typename> typename R>
struct failure_eh final {
  template <cordo::fallible F>
  static constexpr decltype(auto) value(F&& res) noexcept {
    return std::get<0>(((F&&)res).state);
  }
  template <cordo::fallible F>
  static constexpr decltype(auto) error(F&& res) noexcept {
    return std::get<1>(((F&&)res).state);
  }

  template <typename U>
  static constexpr auto ok(U&& res) {
    return res.state.index() == 0;
  }

  template <typename E>
  static constexpr auto empty_error() noexcept {
    return std::optional<E>{};
  }

  template <typename E, cordo::fallible F>
  static constexpr auto make_result(F&& res) noexcept {
    static_assert(
        std::is_same_v<typename std::remove_cvref_t<F>::eh_t, failure_eh>);
    return (F&&)res;
  }
  template <typename E, typename T>
  static constexpr auto make_result(T&& res) noexcept {
    return R<T, E>{(T&&)res};
  }
};

template <typename R, typename E>
struct result final {
  using tag_t = cordo::fallible_tag;
  using eh_t = failure_eh<result>;
  using ok_t = R;
  using err_t = E;

  result(R&& r) : state{(R&&)r} {}
  result(E&& e) : state{(E&&)e} {}

  std::variant<R, E> state;
};

static_assert(cordo::fallible<result<int, std::string>>);

}  // namespace

namespace {

consteval void algo_test() {
  static_assert(algo1("foo") == 1);
  static_assert(algo1(Foo{}) == 2);
  static_assert(algo2(Foo{}) == 3);
  static_assert(algo3(1, 2, 3) == 6);
}

TEST(Algo, Fallible) {
  using R = result<int, std::string>;
  static_assert(cordo::fallible<R>);
  static_assert(
      std::is_same_v<decltype(::cordo_internal_cpo_core::fallible_helpers.eh_t(
                         R{3})),
                     ::cordo::tag_t<failure_eh<result>>>);
  R x = algo3(R(std::string("wow")));
  ASSERT_THAT(x.state.index(), 1);
  EXPECT_THAT(std::get<1>(x.state), ::testing::StrEq("wow"));

  R y = algo3(R{3}, R{4}, 5);
  ASSERT_THAT(y.state.index(), 0);
  EXPECT_THAT(std::get<0>(y.state), ::testing::Eq(12));

  R z = algo3(R{3}, R{"woohoo"}, R{4}, 5);
  ASSERT_THAT(z.state.index(), 1);
  EXPECT_THAT(std::get<1>(z.state), ::testing::StrEq("woohoo"));
}

}  // namespace