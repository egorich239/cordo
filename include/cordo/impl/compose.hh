#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/algo.hh"
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
      operator(), (O outer, I inner),
      (cordo_internal_compose::compose_t<O, I>{outer, inner}));

  template <accessor O, accessor I, accessor... Is>
  CORDO_INTERNAL_LAMBDA_(
      operator(), (O outer, I inner, Is... is),
      ((*this)(cordo_internal_compose::compose_t<O, I>{outer, inner}, is...)));

} compose{};

template <accessor O, accessor I, typename S>
auto cordo_algo(const algo_t<get2_t{}>&, adl_hook_t, S&& s,
                ::cordo_internal_compose::compose_t<O, I> c)
    -> decltype(::cordo::get2(::cordo::get2((S&&)s, c.outer_), c.inner_)) {
  return ::cordo::get2(::cordo::get2((S&&)s, c.outer_), c.inner_);
}

}  // namespace cordo