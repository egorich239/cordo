#pragma once

#include "cordo/impl/accessor.hh"
#include "cordo/impl/algo.hh"
#include "cordo/impl/get2.hh"
#include "cordo/impl/kv.hh"

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

template <::cordo::key_t N, ::cordo::accessor A, typename S>
auto cordo_algo(const algo_t<get2_t{}>&, adl_hook_t, S&& s,
                ::cordo_internal_named::named_t<N, A> n)
    -> decltype(::cordo::get2((S&&)s, n.accessor_)) {
  return ::cordo::get2((S&&)s, n.accessor_);
}

}  // namespace cordo