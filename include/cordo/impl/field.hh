#pragma once

#include <type_traits>

#include "cordo/impl/cpo.hh"
#include "cordo/impl/get2.hh"
#include "cordo/impl/literal.hh"
#include "cordo/impl/macros.hh"

namespace cordo_internal_field {
template <typename S, typename T>
struct field_t final {
  static_assert(!std::is_reference_v<T>);

  using tuple_t = S;
  using value_t = T;
  T S::*field_;
};

}  // namespace cordo_internal_field

namespace cordo {
inline constexpr struct {
  template <typename S, typename T>
  constexpr ::cordo_internal_field::field_t<S, T> operator()(
      T S::*field_) const noexcept {
    return {field_};
  }
} field{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <typename S, typename T>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (const ::cordo::get2_cpo&, adl_tag, S& s,
     ::cordo_internal_field::field_t<S, T> f),  //
    (s.*(f.field_)));

template <typename S, typename T>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (const ::cordo::get2_cpo&, adl_tag, const S& s,
     ::cordo_internal_field::field_t<S, T> f),  //
    (s.*(f.field_)));
}  // namespace cordo_internal_cpo