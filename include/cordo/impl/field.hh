#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/get.hh"

namespace cordo_internal_field {
template <typename S, typename T>
struct field_v final {
  static_assert(!std::is_reference_v<T>);

  using tuple_t = S;
  using value_t = T;
  T S::*field_;
};

struct field_t final {
  template <typename S, typename T>
  constexpr field_v<S, T> operator()(T S::*field_) const noexcept {
    return {field_};
  }
};

}  // namespace cordo_internal_field

namespace cordo {
inline constexpr ::cordo_internal_field::field_t field{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <typename S, typename T>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::get_cpo, adl_tag, ::cordo_internal_field::field_v<S, T> f,
     S& s),  //
    (s.*(f.field_)));

template <typename S, typename T>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::get_cpo, adl_tag, ::cordo_internal_field::field_v<S, T> f,
     const S& s),  //
    (s.*(f.field_)));

template <typename S, typename T>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo_internal_accessor::accessor_implicit_ctor_cpo, adl_tag,
     T(S::*f)),  //
    (::cordo::field(f)));

}  // namespace cordo_internal_cpo