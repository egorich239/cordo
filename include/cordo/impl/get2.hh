#pragma once

#include "cordo/impl/accessor.hh"
#include "cordo/impl/algo.hh"

namespace cordo_internal_get2 {

struct get2_t final {};

}  // namespace cordo_internal_get2

namespace cordo {
using ::cordo_internal_get2::get2_t;

constexpr inline struct {
  template <::cordo::accessor A>
  constexpr auto operator()(const typename A::tuple_t& s, A a) const
      -> decltype(::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a)) {
    return ::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a);
  }
  template <::cordo::accessor A>
  constexpr auto operator()(typename A::tuple_t& s, A a) const
      -> decltype(::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a)) {
    return ::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a);
  }
  template <::cordo::accessor A>
  constexpr void operator()(typename A::tuple_t&& s, A a) const = delete;
} get2{};
}  // namespace cordo