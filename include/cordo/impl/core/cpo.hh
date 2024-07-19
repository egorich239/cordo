#pragma once

#include <cstddef>

#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cpo_core {
template <auto A>
struct cpo_t final {};

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

 public:
  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(), (cpo_t<A> a, Args &&...args) const,
      (this->resolve(::cordo::overload_prio_t<4>{}, a, (Args &&)args...)));
};
}  // namespace cordo_internal_cpo_core

namespace cordo {
using ::cordo_internal_cpo_core::cpo_t;
inline constexpr ::cordo_internal_cpo_core::invoke_t invoke{};
}  // namespace cordo

namespace cordo_internal_cpo {
struct adl_tag final {};
}  // namespace cordo_internal_cpo
