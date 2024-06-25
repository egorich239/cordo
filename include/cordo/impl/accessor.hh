#pragma once

#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cpo {
struct adl_tag final {};
}  // namespace cordo_internal_cpo

namespace cordo_internal_accessor {
template <typename A>
concept accessor = requires {
  requires std::is_default_constructible_v<A>;
  typename ::cordo::value_t<A{}>;

  typename A::tuple_t;
  typename A::const_value_t;
  typename A::mut_value_t;

  requires !std::is_reference_v<typename A::tuple_t> &&
               !std::is_pointer_v<typename A::tuple_t> &&
               !std::is_member_pointer_v<typename A::tuple_t>;
};

struct accessor_implicit_ctor_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  constexpr auto operator()(accessor auto a) const noexcept { return a; }
};
using accessor_implicit_ctor_cpo =
    ::cordo::cpo_t<accessor_implicit_ctor_cpo_t{}>;

struct make_accessor_t final {
  template <typename T>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(),          //
      (T v) const,         //
      (::cordo::invoke(accessor_implicit_ctor_cpo{}, v)));
};
}  // namespace cordo_internal_accessor

namespace cordo {
using ::cordo_internal_accessor::accessor;
inline constexpr ::cordo_internal_accessor::make_accessor_t make_accessor{};
}  // namespace cordo

namespace cordo_internal_cpo {
using ::cordo::accessor;
using ::cordo::cpo_t;
using ::cordo::tag_t;
using ::cordo::typeid_t;
}  // namespace cordo_internal_cpo