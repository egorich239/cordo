#pragma once

#include <string>
#include <type_traits>

#include "cordo/impl/value.hh"

namespace cordo {

template <typename A>
concept accessor = requires {
  requires std::is_default_constructible_v<A>;

  typename A::tuple_t;  // TODO: object_t?
  typename A::value_t;  // TODO: field_t?
  typename ::cordo::value_t<A{}>;

  requires !std::is_reference_v<typename A::tuple_t> &&
               !std::is_pointer_v<typename A::tuple_t> &&
               !std::is_member_pointer_v<typename A::tuple_t>;
};

}  // namespace cordo