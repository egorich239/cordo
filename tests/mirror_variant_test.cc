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
using Foo_map = ::cordo::value_t<("x"_key <= &Foo::x)>;

struct Bar {
  char y;
};
using Bar_map = ::cordo::value_t<("y"_key <= &Bar::y)>;

using Var = std::variant<Foo, Bar>;
using Var_m = ::cordo_internal_mirror::mirror_variant<
    Var, ::cordo::values_t<"Foo"_key, "Bar"_key>>;

}  // namespace