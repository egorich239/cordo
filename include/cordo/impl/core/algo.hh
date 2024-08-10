#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include "cordo/impl/core/invoke.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cpo_core {
template <typename A>
struct algo final {
  static_assert(((void)A{}, true),
                "algorithm traits must be constexpr-constructible");

  using core_t = A;

 private:
  struct trigger final {
    template <typename... Args>
    friend constexpr auto customize(trigger, cordo::overload_prio_t<2>,
                                    const algo &a, Args &&...args)
        CORDO_INTERNAL_ALIAS_(customize(a, (Args &&)args...));
    template <typename... Args>
    friend constexpr auto customize(trigger, cordo::overload_prio_t<1>,
                                    const algo &a, Args &&...args)
        CORDO_INTERNAL_ALIAS_(customize(core_t{}, (Args &&)args...));

    template <typename... Args>
    constexpr auto operator()(cordo::overload_prio_t<0>, const algo &a,
                              Args &&...args) const
        CORDO_INTERNAL_ALIAS_(A{}(a, (Args &&)args...));
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

}  // namespace cordo_internal_cpo_core

namespace cordo {
using ::cordo_internal_cpo_core::algo;
}  // namespace cordo
