#include "cordo/impl/mirror_primitive.hh"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cordo/impl/mirror.hh"

namespace {
using namespace ::cordo::literals;
using ::cordo::cstring;
using ::cordo::make_key;

consteval void mirror_primitive_name_test() {
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    cordo::mirror_traits_ctor(::cordo::tag_t<bool&>{}))),
                make_key<"bool"_cs>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    cordo::mirror_traits_ctor(::cordo::tag_t<char&>{}))),
                make_key<cstring("char")>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    cordo::mirror_traits_ctor(::cordo::tag_t<float&>{}))),
                make_key<cstring("f32")>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    cordo::mirror_traits_ctor(::cordo::tag_t<double&>{}))),
                make_key<cstring("f64")>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    cordo::mirror_traits_ctor(::cordo::tag_t<int32_t&>{}))),
                make_key<cstring("i32")>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    cordo::mirror_traits_ctor(::cordo::tag_t<uint32_t&>{}))),
                make_key<cstring("u32")>>);
}

consteval void mirror_primitive_cpo_test() {
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_ctor(::cordo::tag_t<char&>{})),
                cordo::mirror_primitive<char>>);
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_ctor(::cordo::tag_t<bool&>{})),
                cordo::mirror_primitive<bool>>);
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_ctor(::cordo::tag_t<float&>{})),
                cordo::mirror_primitive<float>>);
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_ctor(::cordo::tag_t<double&>{})),
                cordo::mirror_primitive<double>>);
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_ctor(::cordo::tag_t<int&>{})),
                cordo::mirror_primitive<int>>);
  static_assert(
      std::is_same_v<  //
          decltype(cordo::mirror_traits_ctor(::cordo::tag_t<unsigned&>{})),
          cordo::mirror_primitive<unsigned>>);
}

TEST(MirrorPrimitive, Basic) {
  int x = 3;
  ::cordo::mirror(x) = 5;
  EXPECT_THAT(::cordo::mirror(x).v(), ::testing::Ref(x));
  EXPECT_THAT(x, 5);

  const int cx = 3;
  EXPECT_THAT(::cordo::mirror(cx).v(), ::testing::Ref(cx));

  // TODO: make it work, it will be useful for e.g. class accessors, returning
  // values.
  //
  // auto m = ::cordo::mirror(3);
}

}  // namespace