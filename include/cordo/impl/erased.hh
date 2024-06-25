#pragma once

#include "cordo/impl/get.hh"

namespace cordo_internal_erased {
using ::cordo::value_t;

template <typename S>
struct erased_v final {
  using tuple_t = S;
  using const_getter_t = void* (*)(const S&);
  using mut_getter_t = void* (*)(S&);

  const char* key_;
  const_getter_t const_;
  mut_getter_t mut_;
};

// TODO: this is half-assed implementation, it only reasonably works when get()
// returns a reference, and fails grotesquely with value type returns.
// This should either be detected-and-forbidden, or detected-and-circumvented.
template <typename S, typename A, A a>
struct erased_get_t final {
  static void* const_(const S& s) { return (void*)&::cordo::get(a, s); }
  static void* mut_(S& s) { return (void*)&::cordo::get(a, s); }
};

struct erased_t final {
  template <::cordo::accessor A, A a>
  constexpr decltype(auto) operator()(value_t<a>) const noexcept {
    using tuple_t = typename A::tuple_t;
    using value_t = typename A::value_t;
    using cb_t = erased_get_t<tuple_t, A, a>;
    return erased_v<tuple_t>{&::cordo::typeid_t<value_t>::key, cb_t::const_,
                             cb_t::mut_};
  }
};
}  // namespace cordo_internal_erased

namespace cordo {
inline constexpr ::cordo_internal_erased::erased_t erased{};
}  // namespace cordo

namespace cordo_internal_cpo {
template <typename T, typename S>
T* cordo_cpo(::cordo::get_as_cpo, adl_tag, tag_t<T>,
             ::cordo_internal_erased::erased_v<S> e, S& s) {
  return e.key_ == &typeid_t<T>::key ? (T*)e.mut_(s) : nullptr;
}
template <typename T, typename S>
const T* cordo_cpo(::cordo::get_as_cpo, adl_tag, tag_t<T>,
                   ::cordo_internal_erased::erased_v<S> e, const S& s) {
  return e.key_ == &typeid_t<T>::key ? (const T*)e.const_(s) : nullptr;
}
}  // namespace cordo_internal_cpo