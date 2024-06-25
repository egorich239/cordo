#pragma once

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/get.hh"

namespace cordo_internal_named {
using ::cordo::accessor;
using ::cordo::key_t;

template <key_t N, accessor A>
struct named_v final {
  using tuple_t = typename A::tuple_t;
  using value_t = typename A::value_t;

  constexpr decltype(auto) name() const noexcept { return N(); }
  A accessor_;
};

struct named_t final {
  template <auto K, accessor A>
  constexpr decltype(auto) operator()(key_t<K>, A accessor) const noexcept {
    return named_v<key_t<K>{}, A>{accessor};
  }
};
}  // namespace cordo_internal_named

namespace cordo {
inline constexpr ::cordo_internal_named::named_t named{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <::cordo::key_t N, ::cordo::accessor A, typename S>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::get_cpo, adl_tag, ::cordo_internal_named::named_v<N, A> n,
     S&& s),  //
    (::cordo::get(n.accessor_, (S&&)s)));
}  // namespace cordo_internal_cpo