
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

struct Bar {
  constexpr int bar(int x) const { return x + 4; }
};

template <template <typename, typename> typename R>
struct failure_eh final {
  template <cordo::fallible F>
  static constexpr decltype(auto) as_result(F&& res) noexcept {
    return std::get<0>(((F&&)res).state);
  }
  template <cordo::fallible F>
  static constexpr decltype(auto) as_error(F&& res) noexcept {
    return std::get<1>(((F&&)res).state);
  }

  template <typename U>
  static constexpr auto has_result(U&& res) {
    return res.state.index() == 0;
  }

  template <typename E, cordo::fallible F>
  static constexpr auto make_result(F&& res, cordo::tag_t<E>) noexcept {
    static_assert(std::is_same_v<typename std::remove_cvref_t<F>::error_t, E>);
    static_assert(
        std::is_same_v<typename std::remove_cvref_t<F>::eh_t, failure_eh>);
    return (F&&)res;
  }
  template <typename E, typename T>
  static constexpr auto make_result(T&& res, cordo::tag_t<E>) noexcept {
    return R<T, E>{(T&&)res};
  }
};

template <typename R, typename E>
struct result final {
  using tag_t = cordo::fallible_tag;
  using eh_t = failure_eh<result>;
  using result_t = R;
  using error_t = E;

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

TEST(Algo, Pipe) {
  static_assert((1 | algo3(cordo::piped, 2, 3)) == 6);

  using R = result<int, std::string>;
  R result = R{"noo"} | algo3(cordo::piped, 2, 3);
  ASSERT_THAT(result.state.index(), 1);
  EXPECT_THAT(std::get<1>(result.state), ::testing::StrEq("noo"));

  result = R{6} | algo3(cordo::piped, 2, 3);
  ASSERT_THAT(result.state.index(), 0);
  EXPECT_THAT(std::get<0>(result.state), ::testing::Eq(11));
}

}  // namespace