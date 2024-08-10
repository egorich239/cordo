#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <string_view>

#include "cordo/cordo.hh"

namespace n1 {

struct Cpo {
  int operator()(std::string_view sv) { return sv.size(); }
};

int customize(Cpo, int x) { return x + 3; }

}  // namespace n1

namespace {
struct Foo {
  int member(int x) { return x + 2; }
};

TEST(Invoke, Function) { EXPECT_THAT(cordo::invoke(sin, 0), 0.); }

TEST(Invoke, MemFunction) {
  EXPECT_THAT(cordo::invoke(&Foo::member, Foo{}, 3), 5);
}

TEST(Invoke, Cpo) {
  EXPECT_THAT(cordo::invoke(n1::Cpo{}, "lorem ipsum"), 11);
  EXPECT_THAT(cordo::invoke(n1::Cpo{}, 3), 6);
}

}  // namespace
