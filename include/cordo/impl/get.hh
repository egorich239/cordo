#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_get {
struct get_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
};

using get_cpo = ::cordo::cpo_t<get_cpo_t{}>;

struct get_t final {
  template <::cordo::accessor A>
  CORDO_INTERNAL_LAMBDA_(                         //
      operator(),                                 //
      (A a, const typename A::tuple_t& s) const,  //
      (::cordo::invoke(get_cpo{}, a, s)));

  template <::cordo::accessor A>
  CORDO_INTERNAL_LAMBDA_(                    //
      operator(),                            //
      (A a, typename A::tuple_t & s) const,  //
      (::cordo::invoke(get_cpo{}, a, s)));

  template <::cordo::accessor A>
  constexpr void operator()(A, typename A::tuple_t&&) const = delete;
};
}  // namespace cordo_internal_get

namespace cordo {
using get_cpo = ::cordo_internal_get::get_cpo;
inline constexpr ::cordo_internal_get::get_t get{};
}  // namespace cordo
