#pragma once

#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo {
template <typename A>
concept erased_accessor = requires {
  requires std::is_default_constructible_v<A>;

  typename A::tuple_t;  // TODO: object_t?
  typename ::cordo::value_t<A{}>;

  requires !std::is_reference_v<typename A::tuple_t> &&
               !std::is_pointer_v<typename A::tuple_t> &&
               !std::is_member_pointer_v<typename A::tuple_t>;
};

template <typename A>
concept accessor = requires {
  requires erased_accessor<A>;

  typename A::value_t;  // TODO: field_t?
};
}  // namespace cordo

namespace cordo_internal_cpo {
using ::cordo::accessor;
using ::cordo::cpo_t;
using ::cordo::tag_t;
using ::cordo::typeid_t;
struct adl_tag final {};
}  // namespace cordo_internal_cpo