#pragma once

#include <cstddef>

#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cpo_core {
template <auto A>
struct cpo_t final {};

template <typename A>
struct algo;

struct invoke_t final {
 private:
  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //
      resolve,             //
      (::cordo::overload_prio_t<3>, cpo_t<A> a, Args &&...args) const,
      (cordo_cpo(a, (Args &&)args...)));

  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //
      resolve,             //
      (::cordo::overload_prio_t<2>, cpo_t<A> a, Args &&...args) const,
      (cordo_cpo(a, A.adl_tag(), (Args &&)args...)));
  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //,
      resolve,             //
      (::cordo::overload_prio_t<1>, cpo_t<A>, Args &&...args) const,
      (A((Args &&)args...)));

  template <typename A, typename... Args>
  constexpr auto resolve(::cordo::overload_prio_t<3>, const algo<A> &a,
                         Args &&...args) const
      CORDO_INTERNAL_ALIAS_(customize(a, (Args &&)args...));
  template <typename A, typename... Args>
  constexpr auto resolve(::cordo::overload_prio_t<2>, const algo<A> &a,
                         Args &&...args) const
      CORDO_INTERNAL_ALIAS_(customize(a, typename A::adl_tag{},
                                      (Args &&)args...));
  template <typename A, typename... Args>
  constexpr auto resolve(::cordo::overload_prio_t<1>, const algo<A> &a,
                         Args &&...args) const
      CORDO_INTERNAL_ALIAS_(A{}(a, (Args &&)args...));

 public:
  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(), (cpo_t<A> a, Args &&...args) const,
      (this->resolve(::cordo::overload_prio_t<4>{}, a, (Args &&)args...)));

  template <typename A, typename... Args>
  constexpr auto operator()(const algo<A> &a, Args &&...args) const
      CORDO_INTERNAL_ALIAS_(this->resolve(::cordo::overload_prio_t<4>{}, a,
                                          (Args &&)args...));
};

template <typename A>
struct algo final {
  static_assert(((void)A{}, true),
                "algorithm traits must be constexpr-constructible");

  template <typename... Args>
  constexpr auto operator()(Args &&...args) const  //
      CORDO_INTERNAL_ALIAS_(
          ::cordo_internal_cpo_core::invoke_t{}(*this, (Args &&)args...));

  // TODO: this stop-gap provides some minimum reasonable error-description,
  // and prevents the 1000s lines of gibberish, but maybe we could improve
  // the informativeness of it all?
  constexpr auto operator()(...) const = delete;
};

}  // namespace cordo_internal_cpo_core

namespace cordo {
using ::cordo_internal_cpo_core::algo;
using ::cordo_internal_cpo_core::cpo_t;
inline constexpr ::cordo_internal_cpo_core::invoke_t invoke{};
}  // namespace cordo

namespace cordo_internal_cpo {
struct adl_tag final {};
}  // namespace cordo_internal_cpo
