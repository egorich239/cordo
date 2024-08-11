#pragma once

#include <concepts>
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
  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&...args) const
      noexcept(noexcept(::cordo::invoke(Core, (Args &&)args...))) {
    return ::cordo::invoke(Core, (Args &&)args...);
  }
};

template <typename T>
concept algo = requires(T &&v) {
  { algo_t(v) } -> std::same_as<std::remove_cvref_t<T>>;
};

static_assert(!algo<int>);
static_assert(algo<algo_t<[] {}>>);
}  // namespace cordo_internal_cpo_core

namespace cordo {
using ::cordo_internal_cpo_core::algo;
using ::cordo_internal_cpo_core::algo_t;
}  // namespace cordo
