#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/cpo.hh"
#include "cordo/impl/get2.hh"
#include "cordo/impl/macros.hh"

namespace cordo_internal_compose {
template <::cordo::accessor O, ::cordo::accessor I>
struct compose_t final {
  static_assert(std::is_same_v<typename O::value_t, typename I::tuple_t>);

  using tuple_t = typename O::tuple_t;
  using value_t = typename I::value_t;

  O outer_;
  I inner_;
};
}  // namespace cordo_internal_compose

namespace cordo {
inline constexpr struct {
  template <accessor O, accessor I>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(), (O outer, I inner) const,
      (cordo_internal_compose::compose_t<O, I>{outer, inner}));

  template <accessor O, accessor I, accessor... Is>
  CORDO_INTERNAL_LAMBDA_(
      operator(), (O outer, I inner, Is... is) const,
      ((*this)(cordo_internal_compose::compose_t<O, I>{outer, inner}, is...)));
} compose{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <accessor O, accessor I, typename S>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (const ::cordo::get2_cpo&, adl_tag, S&& s,
     ::cordo_internal_compose::compose_t<O, I> c),  //
    (::cordo::get2(::cordo::get2((S&&)s, c.outer_), c.inner_)));
}  // namespace cordo_internal_cpo