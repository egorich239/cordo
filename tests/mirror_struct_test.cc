#include "cordo/impl/mirror_struct.hh"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

#include "cordo/cordo.hh"

namespace cordo_internal_test {
using namespace ::cordo::literals;
struct SomeStruct {
  int x;
  float y;
  char z;
};

using SomeStruct_map = ::cordo::values_t<  //
    ("x"_key <= &SomeStruct::x),           //
    ("y"_key <= &SomeStruct::y),           //
    ("z"_key <= &SomeStruct::z)>;

constexpr auto customize(decltype(::cordo::mirror_traits_ctor),
                         ::cordo::tag_t<SomeStruct>) noexcept {
  return cordo::mirror_struct<SomeStruct, SomeStruct_map>{};
}
};  // namespace cordo_internal_test

namespace {
using namespace ::cordo::literals;
using ::cordo::cstring;
using ::cordo::make_key;
using ::cordo_internal_test::SomeStruct;
using ::cordo_internal_test::SomeStruct_map;

consteval void mirror_struct_name_test() {
  static_assert(std::is_same_v<  //
                cordo::mirror_struct<SomeStruct, SomeStruct_map>::name,
                make_key<cstring("SomeStruct")>>);
  static_assert(std::is_same_v<  //
                cordo::mirror_struct<const SomeStruct, SomeStruct_map>::name,
                make_key<cstring("SomeStruct")>>);
  static_assert(std::is_same_v<  //
                cordo::mirror_struct<struct Foo, ::cordo::values_t<>>::name,
                make_key<cstring("Foo")>>);
}

consteval void mirror_struct_subscipt_keys_test() {
  using m = cordo::mirror_struct<SomeStruct, SomeStruct_map>;
  static_assert(std::is_same_v<m::subscript_keys,
                               ::cordo::types_t<       //
                                   decltype("x"_key),  //
                                   decltype("y"_key),  //
                                   decltype("z"_key)>>);
}

consteval void mirror_struct_subscipt_map_test() {
  static_assert(::cordo::kv_lookup(SomeStruct_map{}, "x"_key) ==
                &SomeStruct::x);
  static_assert(::cordo::kv_lookup(SomeStruct_map{}, "y"_key) ==
                &SomeStruct::y);
  static_assert(::cordo::kv_lookup(SomeStruct_map{}, "z"_key) ==
                &SomeStruct::z);
}

consteval void mirror_struct_cpo_test() {
  using m = cordo::mirror_struct<SomeStruct, SomeStruct_map>;
  static_assert(std::is_same_v<m, decltype(::cordo::mirror.t(
                                      ::cordo::tag_t<SomeStruct&>{}))>);
}

TEST(MirrorStruct, Mut) {
  SomeStruct f{
      .x = 3,
      .y = .14,
      .z = '1',
  };

  cordo::mirror_api mf = ::cordo::mirror(f);
  EXPECT_THAT(mf.v(), ::testing::Ref(f));

  EXPECT_THAT(mf["x"_key].v(), ::testing::Ref(f.x));
  EXPECT_THAT(mf["y"_key].v(), ::testing::Ref(f.y));
  EXPECT_THAT(mf["z"_key].v(), ::testing::Ref(f.z));

  static_assert(std::is_same_v<typename decltype(mf["x"_key])::traits,
                               cordo::mirror_primitive<int>>);
}

TEST(MirrorStruct, Const) {
  const SomeStruct f{
      .x = 3,
      .y = .14,
      .z = '1',
  };

  cordo::mirror_api mf = ::cordo::mirror(f);
  EXPECT_THAT(mf.v(), ::testing::Ref(f));
  EXPECT_THAT(mf["x"_key].v(), ::testing::Ref(f.x));
  EXPECT_THAT(mf["y"_key].v(), ::testing::Ref(f.y));
  EXPECT_THAT(mf["z"_key].v(), ::testing::Ref(f.z));

  static_assert(std::is_same_v<typename decltype(mf["x"_key])::traits,
                               cordo::mirror_primitive<const int>>);
}

}  // namespace