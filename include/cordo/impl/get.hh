#pragma once

#include <functional>
#include <utility>

#include "composed.hh"
#include "cordo/impl/composed.hh"
#include "cordo/impl/field.hh"
#include "cordo/impl/named.hh"
#include "cordo/impl/property.hh"
#include "cordo/impl/value.hh"

namespace cordo {

template <typename T>
struct type_id_t final {
  static constexpr const char key_{};
};

template <typename S>
struct erased_t final {
  using tuple_t = S;
  using const_getter_t = void* (*)(const S&);
  using mut_getter_t = void* (*)(S&);

  const char* key_;
  const_getter_t const_;
  mut_getter_t mut_;
};

inline constexpr struct {
  // field_t
  template <typename S, typename T>
  constexpr T& operator()(S& s, ::cordo::field_t<S, T> f) const noexcept {
    return s.*(f.field_);
  }

  template <typename S, typename T>
  constexpr T&& operator()(S&& s, ::cordo::field_t<S, T> f) const noexcept {
    return ((S&&)s).*(f.field_);
  }

  template <typename S, typename T>
  constexpr const T& operator()(const S& s,
                                ::cordo::field_t<S, T> f) const noexcept {
    return s.*(f.field_);
  }

  // property_t
  template <typename S, typename T, typename M, typename C>
  constexpr decltype(auto) operator()(
      S& s, ::cordo::property_t<S, T, M, C> f) const noexcept {
    return std::invoke(f.mut_, s);
  }
  template <typename S, typename T, typename M, typename C>
  constexpr decltype(auto) operator()(
      const S& s, ::cordo::property_t<S, T, M, C> f) const noexcept {
    return std::invoke(f.const_, s);
  }

  // composed_t
  template <typename S, typename O, typename I>
  constexpr decltype(auto) operator()(
      const S& s, ::cordo::composed_t<O, I> c) const noexcept {
    return (*this)((*this)(s, c.outer_), c.inner_);
  }

  template <typename S, typename O, typename I>
  constexpr decltype(auto) operator()(
      S& s, ::cordo::composed_t<O, I> c) const noexcept {
    return (*this)((*this)(s, c.outer_), c.inner_);
  }

  // named_t
  template <typename S, auto N, typename A>
  constexpr decltype(auto) operator()(S&& s,
                                      ::cordo::named_t<N, A> f) const noexcept {
    return (*this)(std::forward<S>(s), f.accessor_);
  }

  // erased_t
  template <typename T, typename S>
  constexpr T* as(S& s, ::cordo::erased_t<S> e) const noexcept {
    return e.key_ == &type_id_t<T>::key_ ? (T*)e.mut_(s) : nullptr;
  }
  template <typename T, typename S>
  constexpr const T* as(const S& s, ::cordo::erased_t<S> e) const noexcept {
    return e.key_ == &type_id_t<T>::key_ ? (T*)e.const_(s) : nullptr;
  }
  template <typename T, typename S>
  constexpr const T* as(S&& s, ::cordo::erased_t<S> e) const noexcept = delete;
} get{};

namespace cordo_get_internal {
template <typename S, typename A, A a>
struct get_callback final {
  static void* const_(const S& s) noexcept {
    return (void*)&::cordo::get(s, a);
  }
  static void* mut_(S& s) noexcept { return (void*)&::cordo::get(s, a); }
};
}  // namespace cordo_get_internal

inline constexpr struct {
  template <typename A, A a>
  constexpr decltype(auto) operator()(value_t<a>) const noexcept {
    using tuple_t = typename A::tuple_t;
    using value_t = typename A::value_t;
    using cb_t = ::cordo::cordo_get_internal::get_callback<tuple_t, A, a>;
    return erased_t<tuple_t>{&type_id_t<value_t>::key_, cb_t::const_,
                             cb_t::mut_};
  }
} erased_{};

}  // namespace cordo