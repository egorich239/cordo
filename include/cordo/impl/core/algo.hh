#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include "cordo/impl/core/invoke.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cpo_core {
template <auto Core>
struct algo_t final {
 private:
  struct trigger final {
    template <typename... Args>
    friend constexpr auto customize(trigger, cordo::overload_prio_t<2>,
                                    const algo_t &a, Args &&...args)
        CORDO_INTERNAL_ALIAS_(customize(a, (Args &&)args...));

    template <typename... Args>
    constexpr auto operator()(cordo::overload_prio_t<1>, const algo_t &a,
                              Args &&...args) const
        CORDO_INTERNAL_ALIAS_(cordo::invoke.if_well_formed(Core, a,
                                                           (Args &&)args...));

    template <typename... Args>
    constexpr auto operator()(cordo::overload_prio_t<0>, const algo_t &,
                              Args &&...args) const
        CORDO_INTERNAL_ALIAS_(cordo::invoke.if_well_formed(Core,
                                                           (Args &&)args...));
  };

 public:
  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&...args) const  //
      CORDO_INTERNAL_RETURN_(::cordo::invoke(trigger{},
                                             cordo::overload_prio_t<2>{}, *this,
                                             (Args &&)args...));

  // TODO: this stop-gap provides some minimum reasonable error-description,
  // and prevents the 1000s lines of gibberish, but maybe we could improve
  // the informativeness of it all?
  constexpr auto operator()(...) const = delete;
};

struct algo_concept_impl final {
  template <auto V>
  static constexpr bool test(cordo::tag_t<algo_t<V>>) noexcept {
    return true;
  }
  static constexpr bool test(...) noexcept { return false; }
};

template <typename T>
concept algo = algo_concept_impl::test(cordo::tag_t<std::remove_cvref_t<T>>{});

static_assert(!algo<int>);
static_assert(algo<algo_t<[] {}>>);
}  // namespace cordo_internal_cpo_core

namespace cordo {
using ::cordo_internal_cpo_core::algo;
using ::cordo_internal_cpo_core::algo_t;
}  // namespace cordo
