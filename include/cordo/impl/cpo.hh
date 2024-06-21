#pragma once

#include <cstddef>

#include "cordo/impl/macros.hh"
#include "cordo/impl/meta.hh"

namespace cordo_internal_cpo_core {
template <const auto &A>
struct cpo_t final {};

template <std::size_t N>
struct overload_prio_t : overload_prio_t<N - 1> {};
template <>
struct overload_prio_t<0> {};

struct invoke_t final {
 private:
  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //
      resolve,             //
      (overload_prio_t<3>, const cpo_t<A> &a, Args &&...args) const,
      (cordo_algo(a, (Args &&)args...)));

  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //
      resolve,             //
      (overload_prio_t<2>, const cpo_t<A> &a, Args &&...args) const,
      (cordo_algo(a, A.adl_tag(), (Args &&)args...)));
  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //,
      resolve,             //
      (overload_prio_t<1>, const cpo_t<A> &, Args &&...args) const,
      (A((Args &&)args...)));

 public:
  template <auto A, typename... Args>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(), (const cpo_t<A> &a, Args &&...args) const,
      (this->resolve(overload_prio_t<4>{}, a, (Args &&)args...)));
};
}  // namespace cordo_internal_cpo_core

namespace cordo {
using ::cordo_internal_cpo_core::cpo_t;
inline constexpr ::cordo_internal_cpo_core::invoke_t invoke{};
}  // namespace cordo

namespace cordo_internal_cpo {
using ::cordo::accessor;
using ::cordo::cpo_t;
using ::cordo::tag_t;
using ::cordo::typeid_t;
struct adl_tag final {};
}  // namespace cordo_internal_cpo