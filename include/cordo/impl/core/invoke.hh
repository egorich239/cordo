#pragma once

#include <concepts>
#include <functional>
#include <type_traits>

#include "cordo/impl/core/meta.hh"

namespace cordo {
namespace cordo_internal_invoke {

// Order of invocations:
// - customize(cpo_t<Algo>, args...)
// - for AdlHook in AdlHooks:
//     // shall be installed in the namespace of AdlHook type.
//     customize(cpo_t<Algo, AdlHook>, args...)
// - customize(cpo_t<Algo, decltype(Algo)>, args...) (if Algo is of class type!)
// - Algo(args...)
//
// If any of the customize is missing, they are omitted.
template <auto Algo, typename... AdlHooks>
struct cpo_t final {
  constexpr operator cpo_t<Algo>() const noexcept { return {}; }
};

template <typename T>
concept cpo = requires(T&& v) {
  { cpo_t{v} } -> std::same_as<std::remove_cvref_t<T>>;
};

struct cpo_invoke_impl final {
 private:
  template <typename... Args, std::invocable<Args&&...> Fn>
  constexpr auto apply(cordo::overload_prio_t<5>, Fn&& fn, Args&&... args) const
      noexcept(std::is_nothrow_invocable_v<Fn&&, Args&&...>)
          -> std::invoke_result_t<Fn&&, Args&&...> {
    return std::invoke((Fn&&)fn, (Args&&)args...);
  }

  template <typename... Args, auto Algo, typename... AdlHooks>
  constexpr auto apply(cordo::overload_prio_t<5>, cpo_t<Algo, AdlHooks...>,
                       Args&&... args) const
      noexcept(noexcept(customize(cpo_t<Algo>{}, (Args&&)args...)))
          -> decltype(customize(cpo_t<Algo>{}, (Args&&)args...)) {
    return customize(cpo_t<Algo>{}, (Args&&)args...);
  }

  template <auto Algo, typename AdlHook, typename... AdlHooks, typename... Args>
  constexpr auto apply(cordo::overload_prio_t<4>,
                       cpo_t<Algo, AdlHook, AdlHooks...>, Args&&... args) const
      noexcept(noexcept(customize(cpo_t<Algo, AdlHook>{}, (Args&&)args...)))
          -> decltype(customize(cpo_t<Algo, AdlHook>{}, (Args&&)args...)) {
    return customize(cpo_t<Algo, AdlHook>{}, (Args&&)args...);
  }

  template <auto Algo, typename AdlHook, typename... AdlHooks, typename... Args>
  constexpr auto apply(cordo::overload_prio_t<3>,
                       cpo_t<Algo, AdlHook, AdlHooks...>, Args&&... args) const
      noexcept(noexcept(this->apply(cordo::overload_prio_t<4>{},
                                    cpo_t<Algo, AdlHooks...>{},
                                    (Args&&)args...)))
          -> decltype(this->apply(cordo::overload_prio_t<4>{},
                                  cpo_t<Algo, AdlHooks...>{},
                                  (Args&&)args...)) {
    return this->apply(cordo::overload_prio_t<4>{}, cpo_t<Algo, AdlHooks...>{},
                       (Args&&)args...);
  }

  template <auto Algo, typename... AdlHooks, typename... Args,
            typename AlgoT = std::remove_cvref_t<decltype(Algo)>>
  constexpr auto apply(cordo::overload_prio_t<2>, cpo_t<Algo, AdlHooks...>,
                       Args&&... args) const
      noexcept(noexcept(customize(cpo_t<Algo, AlgoT>{}, (Args&&)args...)))
          -> decltype(customize(cpo_t<Algo, AlgoT>{}, (Args&&)args...))
    requires(std::is_class_v<AlgoT>)
  {
    return customize(cpo_t<Algo, AlgoT>{}, (Args&&)args...);
  }

  template <auto Algo, typename... AdlHooks, typename... Args>
  constexpr auto apply(cordo::overload_prio_t<1>, cpo_t<Algo, AdlHooks...>,
                       Args&&... args) const
      noexcept(std::is_nothrow_invocable_v<decltype(Algo), Args&&...>)
          -> std::invoke_result_t<decltype(Algo), Args&&...> {
    return std::invoke(Algo, (Args&&)args...);
  }

 public:
  template <typename..., typename Fn, typename... Args>
  constexpr auto operator()(Fn&& fn, Args&&... args) const
      noexcept(noexcept(this->apply(cordo::overload_prio_t<5>{}, (Fn&&)fn,
                                    (Args&&)args...)))
          -> decltype(this->apply(cordo::overload_prio_t<5>{}, (Fn&&)fn,
                                  (Args&&)args...)) {
    return this->apply(cordo::overload_prio_t<5>{}, (Fn&&)fn, (Args&&)args...);
  }
};

template <typename Fn, typename... Args>
concept invokable = requires(cpo_invoke_impl invoke_resolved_overload, Fn&& fn,
                             Args&&... args) {
  invoke_resolved_overload((Fn&&)fn, (Args&&)args...);
};

struct cpo_invoke_fn final {
  template <typename..., typename... Args, invokable<Args&&...> Fn>
  constexpr decltype(auto) operator()(Fn&& fn, Args&&... args) const
      noexcept(noexcept(cpo_invoke_impl{}((Fn&&)fn, (Args&&)args...))) {
    return cpo_invoke_impl{}((Fn&&)fn, (Args&&)args...);
  }
};

}  // namespace cordo_internal_invoke

using cordo_internal_invoke::cpo;
using cordo_internal_invoke::cpo_t;
template <auto V, typename... AdlHooks>
inline constexpr cpo_t<V, AdlHooks...> cpo_v;
inline constexpr cordo_internal_invoke::cpo_invoke_fn invoke{};
}  // namespace cordo