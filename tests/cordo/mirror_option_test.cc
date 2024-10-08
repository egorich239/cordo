#include "cordo/impl/mirror_option.hh"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cordo/impl/core/algo.hh>
#include <memory>

#include "cordo/cordo.hh"

namespace {
using namespace cordo::literals;

struct Li final {
  int head;
  std::unique_ptr<Li> tail;
};
using Li_map =
    ::cordo::values_t<("head"_key <= &Li::head), ("tail"_key <= &Li::tail)>;

constexpr auto customize(cordo::hook_t<::cordo::mirror_traits_ctor>,
                         ::cordo::tag_t<Li>) noexcept {
  return cordo::mirror_struct_traits<Li, Li_map>{};
}

TEST(Optional, UniquePtr) {
  std::unique_ptr<int> x = std::make_unique<int>(2);
  cordo::mirror_t m = ::cordo::mirror(x);
  EXPECT_THAT(m.unwrap().v(), ::testing::Ref(*x));
}

TEST(Optional, UniquePtrOfStruct) {
  std::unique_ptr<Li> x = std::make_unique<Li>(Li{.head = 2, .tail = nullptr});
  cordo::mirror_t m = ::cordo::mirror(x);
  Li& s = m.unwrap().v();
  EXPECT_THAT(s, ::testing::Ref(*x));
  EXPECT_THAT(m.unwrap().v(), ::testing::Ref(*x));

  static_assert(std::is_same_v<
                decltype(::cordo::mirror_traits_subscript_keys(
                    typename decltype(m)::traits{})),
                ::cordo::types_t<decltype("head"_key), decltype("tail"_key)>>);
  EXPECT_THAT(m["head"_key].v(), ::testing::Ref(x->head));
  EXPECT_THAT(m["tail"_key].v(), ::testing::Ref(x->tail));
  EXPECT_THAT(m["tail"_key].v(), ::testing::Eq(nullptr));

  m["tail"_key] = std::make_unique<Li>(Li{.head = 3, .tail = nullptr});
  EXPECT_THAT(x->tail, ::testing::Pointee(::testing::Field(&Li::head, 3)));
  EXPECT_THAT(m["head"_key].v(), 2);
  EXPECT_THAT(m["tail"_key]["head"_key].v(), 3);

  const auto& y = x;
  cordo::mirror_t my = ::cordo::mirror(y);
  EXPECT_THAT(my.unwrap().v(), ::testing::Ref(*y));
  EXPECT_THAT(my["head"_key].v(), ::testing::Ref(x->head));
  EXPECT_THAT(my["tail"_key].v(), ::testing::Ref(x->tail));
  EXPECT_THAT(my["tail"_key]["head"_key].v(), 3);
}

TEST(Optional, EhResult) {
  std::unique_ptr<int> x = std::make_unique<int>(2);
  cordo::mirror_t m = ::cordo::mirror(x, cordo::eh_result{});
  ASSERT_TRUE(m.unwrap().ok());
  EXPECT_THAT(m.unwrap().value().v(), ::testing::Ref(*x));
}

}  // namespace
