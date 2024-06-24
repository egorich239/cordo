#pragma once

#include <cstddef>
#include <type_traits>

namespace cordo_internal_meta {

template <typename T>
struct tag_t final {
  using type = T;
};

template <typename... T>
struct types_t final {};

template <auto... V>
struct values_t final {};

template <typename T>
struct typeid_t final {
  static constexpr char key = 0;
};

template <std::size_t N>
struct overload_prio_t : overload_prio_t<N - 1> {};
template <>
struct overload_prio_t<0> {};

template <typename S, typename T>
struct same_constness_as final {
  static_assert(!std::is_reference_v<S> && !std::is_reference_v<T>);

 private:
  template <typename U>
  static constexpr tag_t<const T> impl(tag_t<const U>) noexcept {
    return {};
  }
  template <typename U>
  static constexpr tag_t<T> impl(tag_t<U>) noexcept {
    return {};
  }

 public:
  using type = typename decltype(same_constness_as::impl(tag_t<S>{}))::type;
};

template <typename S, typename T>
using same_constness_as_t = typename same_constness_as<S, T>::type;

}  // namespace cordo_internal_meta

namespace cordo {
using ::cordo_internal_meta::overload_prio_t;
using ::cordo_internal_meta::same_constness_as_t;
using ::cordo_internal_meta::tag_t;
using ::cordo_internal_meta::typeid_t;
using ::cordo_internal_meta::types_t;
using ::cordo_internal_meta::values_t;
}  // namespace cordo