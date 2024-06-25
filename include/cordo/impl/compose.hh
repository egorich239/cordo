#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/get.hh"

namespace cordo_internal_compose {
using ::cordo::accessor;

template <::cordo::accessor O, ::cordo::accessor I>
struct compose_v final {
  static_assert(std::is_same_v<typename O::value_t, typename I::tuple_t>);

  using tuple_t = typename O::tuple_t;
  using value_t = typename I::value_t;

  O outer_;
  I inner_;
};

struct compose_t final {
  template <accessor O, accessor I>
  CORDO_INTERNAL_LAMBDA_(        //
      operator(),                //
      (O outer, I inner) const,  //
      (compose_v<O, I>{outer, inner}));

  template <accessor O, accessor I, accessor... Is>
  CORDO_INTERNAL_LAMBDA_(                  //
      operator(),                          //
      (O outer, I inner, Is... is) const,  //
      ((*this)(compose_v<O, I>{outer, inner}, is...)));
};
}  // namespace cordo_internal_compose

namespace cordo {
inline constexpr ::cordo_internal_compose::compose_t compose{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <accessor O, accessor I, typename S>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::get_cpo, adl_tag, ::cordo_internal_compose::compose_v<O, I> c,
     S&& s),  //
    (::cordo::get(c.inner_, ::cordo::get(c.outer_, (S&&)s))));
}  // namespace cordo_internal_cpo