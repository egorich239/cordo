#pragma once

#include <cstddef>

namespace cordo_internal_meta {

template <typename T>
struct tag_t final {};

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

}  // namespace cordo_internal_meta

namespace cordo {
using ::cordo_internal_meta::tag_t;
using ::cordo_internal_meta::typeid_t;
using ::cordo_internal_meta::types_t;
using ::cordo_internal_meta::overload_prio_t;
using ::cordo_internal_meta::values_t;
}  // namespace cordo