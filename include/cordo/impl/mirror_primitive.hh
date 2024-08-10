#pragma once

#include <climits>
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/mirror.hh"

namespace cordo {
namespace cordo_internal_mirror {
template <typename T>
concept primitive = std::integral<T> || std::floating_point<T>;

inline constexpr struct mirror_primitive_name_t {
  static_assert(CHAR_BIT == 8, "only 8-bit chars are supported");

 private:
  static constexpr const char* NAMES[] = {
      "u8",   "i8",    //
      "u16",  "i16",   //
      "u32",  "i32",   //
      "u64",  "i64",   //
      "u128", "i128",  //
  };

 public:
  constexpr auto operator()(::cordo::tag_t<bool>) const noexcept {
    return ::cordo::cstring("bool");
  }
  constexpr auto operator()(::cordo::tag_t<char>) const noexcept {
    return ::cordo::cstring("char");
  }
  constexpr auto operator()(::cordo::tag_t<float>) const noexcept {
    return ::cordo::cstring("f32");
  }
  constexpr auto operator()(::cordo::tag_t<double>) const noexcept {
    return ::cordo::cstring("f64");
  }
  template <std::integral T>
  constexpr auto operator()(::cordo::tag_t<T>) const noexcept {
    constexpr auto suffix =
        ::cordo::to_cstring(::cordo::value_t<(size_t)(CHAR_BIT * sizeof(T))>{});
    if constexpr (std::is_signed_v<T>) {
      return ::cordo::cstring("i").concat(suffix);
    } else {
      return ::cordo::cstring("u").concat(suffix);
    }
  }
  constexpr auto operator()(...) const noexcept { return ::cordo::null_t{}; }
} mirror_primitive_name{};

template <primitive T>
struct mirror_primitive final {
  using t = T;
  using rep = T&;
  using name = std::conditional_t<
      !std::is_same_v<decltype(mirror_primitive_name(::cordo::tag_t<T>{})),
                      ::cordo::null_t>,
      cordo::make_key<mirror_primitive_name(::cordo::tag_t<T>{})>,
      ::cordo::null_t>;
  using subscript_keys = ::cordo::null_t;
};

static_assert(std::is_same_v<typename mirror_primitive<int>::name,
                             cordo::make_key<::cordo::cstring("i32")>>);

template <primitive T>
constexpr auto customize(mirror_traits_ctor_core_t,
                         ::cordo::tag_t<T>) noexcept {
  return mirror_primitive<T>{};
}

template <primitive T>
constexpr auto customize(mirror_traits_of_const_core_t,
                         mirror_primitive<T>) noexcept {
  return mirror_primitive<const T>{};
}

}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_primitive;
}  // namespace cordo