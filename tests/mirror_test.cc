#include "cordo/impl/mirror.hh"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/meta.hh"

namespace {
using namespace ::cordo::literals;
using ::cordo::cstring;
using ::cordo::make_key;

struct NamedTrait_foo {
  using name = ::cordo::make_key<"foo"_cs>;
};

struct NamedTrait_null {
  using name = ::cordo::null_t;
};

struct NamedTrait_no {};

consteval void mirror_name_test() {
  constexpr NamedTrait_foo nt_foo{};
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_name(NamedTrait_foo{})),
                make_key<"foo"_cs>>);
  static_assert(
      std::is_same_v<  //
          decltype(cordo::mirror_traits_name(nt_foo)), make_key<"foo"_cs>>);

  constexpr NamedTrait_null nt_null{};
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_name(NamedTrait_null{})),
                ::cordo::null_t>);
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_name(nt_null)), ::cordo::null_t>);

  constexpr NamedTrait_no nt_no{};
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_name(NamedTrait_no{})),
                ::cordo::null_t>);
  static_assert(std::is_same_v<  //
                decltype(cordo::mirror_traits_name(nt_no)), ::cordo::null_t>);
}

}  // namespace