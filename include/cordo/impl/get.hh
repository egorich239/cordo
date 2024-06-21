#pragma once

#include <functional>
#include <utility>

#include "cordo/impl/kv.hh"
#include "cordo/impl/property.hh"
#include "cordo/impl/get2.hh"

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
    return (void*)&::cordo::get2(s, a);
  }
  static void* mut_(S& s) noexcept { return (void*)&::cordo::get2(s, a); }
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