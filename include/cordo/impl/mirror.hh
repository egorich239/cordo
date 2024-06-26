#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/macros.hh"

namespace cordo_internal_mirror {

struct mirror_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
};

struct mirror_construct_cpo_t final {
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
      (::cordo::get(
          ::cordo::make_accessor(::cordo::kv_lookup(typename T::fields_t{}, k)),
          s)));
};

using mirror_cpo = ::cordo::cpo_t<mirror_cpo_t{}>;
using mirror_construct_cpo = ::cordo::cpo_t<mirror_construct_cpo_t{}>;
using mirror_subscript_cpo = ::cordo::cpo_t<mirror_subscript_cpo_t{}>;

struct mirror_t final {
  template <typename S>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(),          //
      (S& v) const,        //
      (::cordo::invoke(
          mirror_construct_cpo{},
          ::cordo::invoke(mirror_cpo{},
                          ::cordo::tag_t<std::remove_cvref_t<S>>{}),
          v)));
};
}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirror_construct_cpo;
using ::cordo_internal_mirror::mirror_cpo;
using ::cordo_internal_mirror::mirror_subscript_cpo;
inline constexpr ::cordo_internal_mirror::mirror_t mirror{};
}  // namespace cordo

// namespace cordo_internal_mirror {

// template <typename S, auto T>  // TODO: named tuple
// struct mirror_t final {
//   S& v;

//   template <typename K>
//   CORDO_INTERNAL_LAMBDA_(  //
//       operator[],          //
//       (K k) const,         //
//       (::cordo::invoke(::cordo::mirror_subscript_cpo{}, T, k)));
// };

// inline constexpr struct {
//   template <typename S>
//   constexpr decltype(auto) operator()(S& v) const noexcept {
//     constexpr auto Tuple = ::cordo::invoke(
//         ::cordo::mirror_cpo{}, ::cordo::tag_t<std::remove_cvref_t<S>>{});
//     return mirror_t<S, Tuple>{v};
//   }
// } mirror_impl;

// }  // namespace cordo_internal_mirror

// namespace cordo_internal_cpo {

// inline constexpr struct {
//   template <typename >
//   CORDO_INTERNAL_LAMBDA_(  //
//       operator(),          //
//       (const S& v) const,  //
//       (::cordo_internal_mirror::mirror_impl(v)));
// } mirror{};

// }  // namespace cordo