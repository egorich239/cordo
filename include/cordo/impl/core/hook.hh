#pragma once

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/invoke.hh"

namespace cordo {
namespace cordo_internal_hook {

struct hook_impl_t final {
  template <auto Algo, typename... AdlHooks>
  static constexpr cpo_t<Algo> impl(cpo_t<Algo, AdlHooks...>) noexcept {
    return {};
  }
  template <cpo C, C p>
  static constexpr auto impl(algo_t<p>) noexcept {
    return hook_impl_t::impl(p);
  }
};

template <auto V>
using hook_t = decltype(hook_impl_t::impl(V));

}  // namespace cordo_internal_hook

using cordo_internal_hook::hook_t;
}  // namespace cordo