#pragma once

#include "cordo/impl/accessor.hh"
#include "cordo/impl/cpo.hh"
#include "cordo/impl/get2.hh"
#include "cordo/impl/kv.hh"
#include "cordo/impl/macros.hh"

namespace cordo_internal_named {
template <::cordo::key_t N, ::cordo::accessor A>
struct named_t final {
  using tuple_t = typename A::tuple_t;
  using value_t = typename A::value_t;

  constexpr decltype(auto) name() const noexcept { return N(); }
  A accessor_;
};
}  // namespace cordo_internal_named

namespace cordo {
inline constexpr struct {
  template <auto K, accessor A>
  constexpr decltype(auto) operator()(key_t<K>, A accessor) const noexcept {
    return ::cordo_internal_named::named_t<key_t<K>{}, A>{accessor};
  }
} named{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <::cordo::key_t N, ::cordo::accessor A, typename S>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_algo,          //
    (const ::cordo::get2_cpo&, adl_tag, S&& s,
     ::cordo_internal_named::named_t<N, A> n),  //
    (::cordo::get2((S&&)s, n.accessor_)));
}  // namespace cordo_internal_cpo