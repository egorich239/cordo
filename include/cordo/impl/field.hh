#pragma once

#include <type_traits>

#include "cordo/impl/algo.hh"
#include "cordo/impl/get2.hh"
#include "cordo/impl/literal.hh"

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

template <typename S, typename T>
decltype(auto) cordo_algo(const algo_t<get2_t{}>&, adl_hook_t, const S& s,
                          ::cordo_internal_field::field_t<S, T> f) noexcept {
  return s.*(f.field_);
}

template <typename S, typename T>
decltype(auto) cordo_algo(const algo_t<get2_t{}>&, adl_hook_t, S& s,
                          ::cordo_internal_field::field_t<S, T> f) noexcept {
  return s.*(f.field_);
}

}  // namespace cordo