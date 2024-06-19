#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

namespace cordo {
namespace cordo_property_internal {
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
}  // namespace cordo_property_internal

template <typename S, typename T,
          ::cordo::cordo_property_internal::property_mut_or_const<S, T> Mut,
          ::cordo::cordo_property_internal::property_const<S, T> Const>
struct property_t final {
  using tuple_t = S;
  using value_t = T;

  Mut mut_;
  Const const_;
};

inline constexpr struct {
  template <typename S, typename T,
            ::cordo::cordo_property_internal::property_mut<S, T> Mut,
            ::cordo::cordo_property_internal::property_const<S, T> Const>
  constexpr decltype(auto) operator()(Const const_, Mut mut_) const noexcept {
    return property_t<S, T, Mut, Const>{mut_, const_};
  }

  template <typename S, typename T>
  constexpr decltype(auto) operator()(const T& (S::*const_)() const,
                                      T& (S::*mut_)()) const noexcept {
    using Mut = T& (S::*)();
    using Const = const T& (S::*)() const;
    return property_t<S, T, Mut, Const>{mut_, const_};
  }

  template <typename S, typename T>
  constexpr decltype(auto) operator()(T (S::*const_)() const,
                                      T& (S::*mut_)()) const noexcept {
    using Mut = T& (S::*)();
    using Const = T (S::*)() const;
    return property_t<S, T, Mut, Const>{mut_, const_};
  }

  template <typename S, typename T,
            ::cordo::cordo_property_internal::property_const<S, T> Const>
  constexpr decltype(auto) operator()(Const const_) const noexcept {
    return property_t<S, const T, Const, Const>{const_, const_};
  }

  template <typename S, typename T>
  constexpr decltype(auto) operator()(T (S::*const_)() const) const noexcept {
    using Const = T (S::*)() const;
    return property_t<S, const std::remove_cvref_t<T>, Const, Const>{const_,
                                                                     const_};
  }
} property_{};

}  // namespace cordo