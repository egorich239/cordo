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
                ::cordo_internal_mirror::mirror_primitive<bool>::name,
                make_key<cstring("bool")>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    ::cordo_internal_mirror::mirror_primitive<bool>{})),
                make_key<"bool"_cs>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror_traits_name(
                    ::cordo::mirror.t(::cordo::tag_t<bool&>{}))),
                make_key<"bool"_cs>>);

  static_assert(std::is_same_v<  //
                ::cordo_internal_mirror::mirror_primitive<char>::name,
                make_key<cstring("char")>>);
  static_assert(std::is_same_v<  //
                ::cordo_internal_mirror::mirror_primitive<float>::name,
                make_key<cstring("f32")>>);
  static_assert(std::is_same_v<  //
                ::cordo_internal_mirror::mirror_primitive<double>::name,
                make_key<cstring("f64")>>);
  static_assert(std::is_same_v<  //
                ::cordo_internal_mirror::mirror_primitive<int32_t>::name,
                make_key<cstring("i32")>>);
  static_assert(std::is_same_v<  //
                ::cordo_internal_mirror::mirror_primitive<uint32_t>::name,
                make_key<cstring("u32")>>);
}

consteval void mirror_primitive_cpo_test() {
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror.t(::cordo::tag_t<char&>{})),
                ::cordo_internal_mirror::mirror_primitive<char>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror.t(::cordo::tag_t<bool&>{})),
                ::cordo_internal_mirror::mirror_primitive<bool>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror.t(::cordo::tag_t<float&>{})),
                ::cordo_internal_mirror::mirror_primitive<float>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror.t(::cordo::tag_t<double&>{})),
                ::cordo_internal_mirror::mirror_primitive<double>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror.t(::cordo::tag_t<int&>{})),
                ::cordo_internal_mirror::mirror_primitive<int>>);
  static_assert(std::is_same_v<  //
                decltype(::cordo::mirror.t(::cordo::tag_t<unsigned&>{})),
                ::cordo_internal_mirror::mirror_primitive<unsigned>>);
}

TEST(MirrorPrimitive, Basic) {
  int x = 3;
  ::cordo::mirror(x) = 5;
  EXPECT_THAT(::cordo::mirror(x).v(), ::testing::Ref(x));
  EXPECT_THAT(x, 5);

  const int cx = 3;
  EXPECT_THAT(::cordo::mirror(cx).v(), ::testing::Ref(cx));
}

}  // namespace