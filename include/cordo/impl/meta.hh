#pragma once

namespace cordo_internal_meta {

template <typename T>
struct tag_t final {};

template <typename... T>
struct types_t final {};

template <typename T>
struct typeid_t final {
  static constexpr char key = 0;
};

}  // namespace cordo_internal_meta

namespace cordo {
using ::cordo_internal_meta::tag_t;
using ::cordo_internal_meta::typeid_t;
using ::cordo_internal_meta::types_t;
}  // namespace cordo