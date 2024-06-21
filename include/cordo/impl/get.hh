#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/cpo.hh"
#include "cordo/impl/macros.hh"
#include "cordo/impl/meta.hh"

namespace cordo_internal_get {
struct get_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
};

struct get_as_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
  template <typename T, ::cordo::accessor A, typename S>
  CORDO_INTERNAL_LAMBDA_R_(                                    //
      operator(),                                              //
      (::cordo::tag_t<T>, S&& s, A a) const,                   //
      (::cordo::invoke(::cordo::cpo_t<get_cpo_t{}>{}, s, a)),  //
      requires(std::is_same_v<T, std::remove_cvref_t<typename A::value_t>>));
};
}  // namespace cordo_internal_get

namespace cordo {
using get_cpo = cpo_t<::cordo_internal_get::get_cpo_t{}>;
using get_as_cpo = cpo_t<::cordo_internal_get::get_as_cpo_t{}>;

inline constexpr struct {
  template <::cordo::accessor A>
  CORDO_INTERNAL_LAMBDA_(                         //
      operator(),                                 //
      (const typename A::tuple_t& s, A a) const,  //
      (::cordo::invoke(get_cpo{}, s, a)));

  template <::cordo::accessor A>
  CORDO_INTERNAL_LAMBDA_(                   //
      operator(),                           //
      (typename A::tuple_t& s, A a) const,  //
      (::cordo::invoke(get_cpo{}, s, a)));

  template <::cordo::accessor A>
  constexpr void operator()(typename A::tuple_t&& s, A a) const = delete;

  template <typename T, ::cordo::erased_accessor A>
  CORDO_INTERNAL_LAMBDA_(                         //
      as,                                         //
      (const typename A::tuple_t& s, A a) const,  //
      (::cordo::invoke(get_as_cpo{}, ::cordo::tag_t<T>{}, s, a)));
  template <typename T, ::cordo::erased_accessor A>
  CORDO_INTERNAL_LAMBDA_(                   //
      as,                                   //
      (typename A::tuple_t& s, A a) const,  //
      (::cordo::invoke(get_as_cpo{}, ::cordo::tag_t<T>{}, s, a)));
  template <typename T, ::cordo::erased_accessor A>
  constexpr void as(typename A::tuple_t&& s, A a) const = delete;
} get{};
}  // namespace cordo
