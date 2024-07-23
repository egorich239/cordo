#include "cordo/impl/mirror_variant.hh"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <variant>

#include "cordo/cordo.hh"

namespace {
using namespace ::cordo::literals;

struct Foo {
  int x;
};
using Foo_map = ::cordo::values_t<("x"_key <= &Foo::x)>;

struct Bar {
  char y;
};
using Bar_map = ::cordo::values_t<("y"_key <= &Bar::y)>;

using Var = std::variant<Foo, Bar>;
using Var_m = ::cordo_internal_mirror::mirror_variant<
    Var, ::cordo::values_t<"Foo"_key, "Bar"_key>>;

constexpr auto customize(decltype(::cordo::mirror_traits_ctor),
                         ::cordo::tag_t<Foo> v) noexcept {
  return ::cordo_internal_mirror::mirror_struct<Foo, Foo_map>{};
}
constexpr auto customize(decltype(::cordo::mirror_traits_ctor),
                         ::cordo::tag_t<Bar> v) noexcept {
  return cordo_internal_mirror::mirror_struct<Bar, Bar_map>{};
}
constexpr auto customize(decltype(::cordo::mirror_traits_ctor),
                         ::cordo::tag_t<Var> v) noexcept {
  return Var_m{};
}

TEST(Variant, Basic) {
  Var x = Foo{.x = 1};
  cordo::mirror_api m = cordo::mirror(x);
  auto mt = cordo::mirror.traits(x);

  static_assert(std::is_same_v<decltype(mt)::subscript_map,
                               ::cordo::values_t<("Foo"_key <= (size_t)0),
                                                 ("Bar"_key <= (size_t)1)>>);
  cordo::mirror_api mfoo = m["Foo"_key];
  static_assert(std::is_same_v<decltype(::cordo::mirror_traits_subscript_keys(
                                   decltype(mfoo)::traits{})),
                               cordo::types_t<decltype("x"_key)>>);
  EXPECT_THAT(mfoo["x"_key].v(), ::testing::Ref(std::get<Foo>(x).x));

  const Var& y = x;
  cordo::mirror_api my = cordo::mirror(y);
  EXPECT_THAT(my["Foo"_key].unwrap().v(), ::testing::Ref(std::get<Foo>(y)));
  EXPECT_THAT(my["Foo"_key]["x"_key].v(), ::testing::Ref(std::get<Foo>(y).x));

  mfoo = Foo{.x = 4};
  EXPECT_THAT(my["Foo"_key]["x"_key].v(), 4);

  x = Bar{.y = 5};
  EXPECT_THAT(my["Bar"_key]["y"_key].v(), 5);

  mfoo = Foo{.x = 6};
  EXPECT_THAT(std::get<Foo>(x).x, 6);
  EXPECT_THAT(my["Foo"_key]["x"_key].v(), 6);
}

}  // namespace