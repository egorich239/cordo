#pragma once

#include "cordo/impl/cpo.hh"
#include "cordo/impl/macros.hh"

namespace cordo_internal_mirror {

struct mirror_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
};

struct mirror_subscript_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  template <typename S, typename T, typename K>
  CORDO_INTERNAL_LAMBDA_(       //
      operator(),               //
      (S&& s, T t, K k) const,  //
      (::cordo::get(s, t[k])));
};

}  // namespace cordo_internal_mirror

namespace cordo {
using mirror_cpo = cpo_t<::cordo_internal_mirror::mirror_cpo_t{}>;
using mirror_subscript_cpo =
    cpo_t<::cordo_internal_mirror::mirror_subscript_cpo_t{}>;
}  // namespace cordo

namespace cordo_internal_mirror {

template <typename S, auto T>  // TODO: named tuple
struct mirror_t final {
  S& v;

  template <typename K>
  CORDO_INTERNAL_LAMBDA_(  //
      operator[],          //
      (K k) const,         //
      (::cordo::invoke(::cordo::mirror_subscript_cpo{}, T, k)));
};

inline constexpr struct {
  template <typename S>
  constexpr decltype(auto) operator()(S& v) const noexcept {
    constexpr auto Tuple = ::cordo::invoke(
        ::cordo::mirror_cpo{}, ::cordo::tag_t<std::remove_cvref_t<T>>{});
    return mirror_t<S, Tuple>{v};
  }
} mirror_impl;

}  // namespace cordo_internal_mirror

namespace cordo {
using mirror_cpo = cpo_t<::cordo_internal_mirror::mirror_cpo_t>;

inline constexpr struct {
  template <typename T>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(),          //
      (const S& v) const,  //
      (::cordo_internal_mirror::mirror_impl(v)));
} mirror{};

}  // namespace cordo