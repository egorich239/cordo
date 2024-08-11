#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <type_traits>

#include "cordo/cordo.hh"

namespace {

template <typename T>
struct Res {
  std::variant<T, std::string> data;

  template <typename U>
  Res(U&& v) : data{(U&&)v} {}
};

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_has_value),
                                   const Res<T>& r) noexcept {
  return r.data.index() == 0;
}

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_value),
                                   const Res<T>& r) noexcept {
  return std::get<0>(r.data);
}

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_value),
                                   Res<T>& r) noexcept {
  return std::get<0>(r.data);
}

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_error),
                                   const Res<T>& r) noexcept {
  return std::get<1>(r.data);
}

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_error),
                                   Res<T>& r) noexcept {
  return std::get<1>(r.data);
}

struct eh final {
  template <typename T, typename U>
  static auto make_result(Res<T>&& v, cordo::tag_t<U>)
    requires(std::is_constructible_v<std::string, U &&>)
  {
    return (Res<T>&&)v;
  }

  template <typename T, typename U>
  static auto make_result(T&& v, cordo::tag_t<U>)
    requires(std::is_constructible_v<std::string, U &&>)
  {
    return Res<T>{(T&&)v};
  }

  template <typename U>
  static auto make_error(U&& v)
    requires(std::is_constructible_v<std::string, U &&>)
  {
    return std::string((U&&)v);
  }
};

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_factory),
                                   const Res<T>&) noexcept {
  return eh{};
}

static_assert(cordo::fallible<Res<int>>);
static_assert(!cordo::fallible<int>);

TEST(Pipe, Then) {
  EXPECT_THAT(3 | cordo::piped([](int x, int y) { return x + y; }, 5), 8);

  Res r = Res<int>{3} | cordo::piped([](int x) { return x + 2; });
  ASSERT_TRUE(cordo::invoke(cordo::fallible_has_value, r));
  EXPECT_THAT(cordo::invoke(cordo::fallible_get_value, r),
              testing::Ref(std::get<0>(r.data)));
  EXPECT_THAT(cordo::invoke(cordo::fallible_get_value, r), 5);

  r = Res<int>{std::string("fail")} | cordo::piped([](int x) { return x + 2; });
  ASSERT_FALSE(cordo::invoke(cordo::fallible_has_value, r));
  EXPECT_THAT(cordo::invoke(cordo::fallible_get_error, r),
              testing::Ref(std::get<1>(r.data)));
  EXPECT_THAT(cordo::invoke(cordo::fallible_get_error, r),
              testing::StrEq("fail"));
}

}  // namespace