#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cordo/impl/core/literal.hh>
#include <cstddef>
#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/mirror.hh"

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
    return "bool";
  }
  constexpr auto operator()(::cordo::tag_t<char>) const noexcept {
    return "char";
  }
  constexpr auto operator()(::cordo::tag_t<float>) const noexcept {
    return "f32";
  }
  constexpr auto operator()(::cordo::tag_t<double>) const noexcept {
    return "f64";
  }
  template <std::integral T>
  constexpr auto operator()(::cordo::tag_t<T>) const noexcept {
    if constexpr (std::bit_ceil(sizeof(T)) != sizeof(T)) {
      return nullptr;
    } else if constexpr (2 * std::bit_width(sizeof(T)) * sizeof(char*) >=
                         sizeof(NAMES)) {
      return nullptr;
    } else {
        constexpr std::string_view name = NAMES[2 * std::bit_width(sizeof(T)) - (std::is_signed_v<T> ? 1 : 2)];
      return ::cordo_internal_literal::string_v<>(
          );
    }
  }
} mirror_primitive_name{};

template <primitive T>
struct mirror_primitive final {
  using t = T;
  using name = std::conditional_t<
      !std::is_same_v<decltype(mirror_primitive_name(::cordo::tag_t<T>{})),
                      std::nullptr_t>,
      cordo::make_key<mirror_primitive_name(::cordo::tag_t<T>{})>,
      ::cordo_internal_meta::null_t>;
};

static_assert(
    std::is_same_v<typename mirror_primitive<int>::name, decltype("i32"_key)>);

}  // namespace cordo_internal_mirror

namespace cordo_internal_cpo {

template <::cordo_internal_mirror::primitive T>
constexpr auto cordo_cpo(::cordo::mirror_traits_cpo, adl_tag,
                         ::cordo::tag_t<T>) noexcept {
  return ::cordo_internal_mirror::mirror_primitive<T>{};
}

}  // namespace cordo_internal_cpo