#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/macros.hh"

namespace cordo_internal_property {
template <typename F, typename S, typename T>
concept property_mut = requires(const F& f, S& s) {
  { std::invoke(f, s) } -> std::same_as<T&>;
};
template <typename F, typename S, typename T>
concept property_const = requires(const F& f, const S& s) {
  requires std::is_same_v<const T&, decltype(std::invoke(f, s))> ||
               std::is_same_v<std::remove_const_t<T>,
                              decltype(std::invoke(f, s))>;
};
template <typename F, typename S, typename T>
concept property_mut_or_const =
    property_const<F, S, T> || property_mut<F, S, T> ||
    property_mut<F, S, const T>;

template <typename S, typename T, property_mut_or_const<S, T> Mut,
          property_const<S, T> Const>
struct property_v final {
  using tuple_t = S;
  using value_t = T;

  Mut mut_;
  Const const_;
};

struct property_t final {
  template <typename S, typename T,
            ::cordo_internal_property::property_mut<S, T> Mut,
            ::cordo_internal_property::property_const<S, T> Const>
  constexpr decltype(auto) operator()(Const const_, Mut mut_) const noexcept {
    return property_v<S, T, Mut, Const>{mut_, const_};
  }

  template <typename S, typename T>
  constexpr decltype(auto) operator()(const T& (S::*const_)() const,
                                      T& (S::*mut_)()) const noexcept {
    using Mut = T& (S::*)();
    using Const = const T& (S::*)() const;
    return property_v<S, T, Mut, Const>{mut_, const_};
  }

  template <typename S, typename T>
  constexpr decltype(auto) operator()(T (S::*const_)() const,
                                      T& (S::*mut_)()) const noexcept {
    using Mut = T& (S::*)();
    using Const = T (S::*)() const;
    return property_v<S, T, Mut, Const>{mut_, const_};
  }

  template <typename S, typename T,
            ::cordo_internal_property::property_const<S, T> Const>
  constexpr decltype(auto) operator()(Const const_) const noexcept {
    return property_v<S, const T, Const, Const>{const_, const_};
  }

  template <typename S, typename T>
  constexpr decltype(auto) operator()(T (S::*const_)() const) const noexcept {
    using Const = T (S::*)() const;
    return property_v<S, const std::remove_cvref_t<T>, Const, Const>{const_,
                                                                     const_};
  }
};
}  // namespace cordo_internal_property

namespace cordo {

inline constexpr ::cordo_internal_property::property_t property{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <typename S, typename T,
          ::cordo_internal_property::property_mut_or_const<S, T> Mut,
          ::cordo_internal_property::property_const<S, T> Const>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::get_cpo, adl_tag,
     ::cordo_internal_property::property_v<S, T, Mut, Const> p,
     const typename ::cordo_internal_property::property_v<S, T, Mut,
                                                          Const>::tuple_t&
         s),  //
    (std::invoke(p.const_, s)));

template <typename S, typename T,
          ::cordo_internal_property::property_mut_or_const<S, T> Mut,
          ::cordo_internal_property::property_const<S, T> Const>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::get_cpo, adl_tag,
     ::cordo_internal_property::property_v<S, T, Mut, Const> p,
     typename ::cordo_internal_property::property_v<S, T, Mut,
                                                    Const>::tuple_t& s),  //
    (std::invoke(p.mut_, s)));
}  // namespace cordo_internal_cpo