#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cordo/cordo.hh>
#include <cordo_json/cordo_json.hh>
#include <optional>

namespace {
using nlohmann::json;
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

TEST(ToJson, Primitive) {
  int x = 3;
  json j;
  cordo_json::to_json(cordo::mirror(x), j);
  ASSERT_TRUE(j.is_number());
  EXPECT_THAT(j.get<int>(), 3);
}

TEST(ToJson, Optional) {
  std::optional<float> x = 3.14;
  json j;

  cordo_json::to_json(cordo::mirror(x), j);
  ASSERT_TRUE(j.is_number());
  EXPECT_THAT(j.get<float>(), testing::FloatNear(3.14, 1e-6));

  // Verify that conversion works with eh_result!
  x = 2.71;
  cordo_json::to_json(cordo::mirror(x, cordo::eh_result{}), j);
  ASSERT_TRUE(j.is_number());
  EXPECT_THAT(j.get<float>(), testing::FloatNear(2.71, 1e-6));

  x = std::nullopt;
  cordo_json::to_json(cordo::mirror(x), j);
  EXPECT_TRUE(j.is_null());
}

TEST(ToJson, Struct) {
  Li x{3, std::make_unique<Li>(Li{5, nullptr})};
  json j;
  cordo_json::to_json(cordo::mirror(x), j);
  EXPECT_THAT(j, testing::Eq(json{
                     {"head", 3},
                     {"tail", {{"head", 5}, {"tail", nullptr}}},
                 }));
}

}  // namespace