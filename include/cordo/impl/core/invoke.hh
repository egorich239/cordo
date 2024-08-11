#pragma once

#include <type_traits>

#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo {
namespace cordo_internal_invoke {

struct invoke_fn final {
 private:
  template <typename A, typename... Args>
  constexpr auto resolve(::cordo::overload_prio_t<2>, A &&a,
                         Args &&...args) const
      noexcept(noexcept(customize((A &&)a, (Args &&)args...)))
          -> decltype(customize((A &&)a, (Args &&)args...))
    requires(!std::is_function_v<A> && !std::is_member_function_pointer_v<A>)
  {
    return customize((A &&)a, (Args &&)args...);
  }

  template <typename A, typename... Args>
  constexpr auto resolve(::cordo::overload_prio_t<1>, A &&a,
                         Args &&...args) const
      CORDO_INTERNAL_ALIAS_(std::invoke((A &&)a, (Args &&)args...));

 public:
  template <typename A, typename... Args>
  constexpr decltype(auto) operator()(A &&a, Args &&...args) const
      CORDO_INTERNAL_RETURN_(this->resolve(::cordo::overload_prio_t<3>{},
                                           (A &&)a, (Args &&)args...));

  template <typename A, typename... Args>
  constexpr auto if_well_formed(A &&a, Args &&...args) const
      CORDO_INTERNAL_ALIAS_(this->resolve(::cordo::overload_prio_t<3>{},
                                          (A &&)a, (Args &&)args...));
};
inline constexpr invoke_fn invoke{};
}  // namespace cordo_internal_invoke

using cordo_internal_invoke::invoke;
}  // namespace cordo