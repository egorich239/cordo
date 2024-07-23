#pragma once

#include <cstddef>
#include <type_traits>

#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cpo_core {
struct fallible_tag final {};

template <typename T, typename EH>
concept fallible = requires {
  requires std::is_same_v<typename T::tag_t, fallible_tag>;
  typename T::ok_t;
  typename T::err_t;
};

inline constexpr struct {
  template <typename EH, fallible<EH> F>
  constexpr auto err1_t(F &&) const noexcept {
    return ::cordo::types_t<typename F::err_t>{};
  }

  template <typename T>
  constexpr auto err1_t(T &&) const noexcept {
    return ::cordo::types_t<>{};
  }

  template <typename EH, fallible<EH> F>
  constexpr auto eh1_t(F &&) const noexcept {
    return ::cordo::types_t<typename F::eh_t>{};
  }

  template <typename T>
  constexpr auto eh1_t(T &&) const noexcept {
    return ::cordo::types_t<>{};
  }

  constexpr auto common_t(cordo::types_t<>) const noexcept {
    return cordo::null_t{};
  }
  template <typename... E>
  constexpr auto common_t(cordo::types_t<E...>) const noexcept {
    return ::cordo::tag_t<std::common_type_t<E...>>{};
  }

  template <typename... Args>
  constexpr auto common_err_t(Args &&...a) const noexcept {
    return this->common_t(::cordo::meta.concat(this->err1_t((Args &&)a)...));
  }

  template <typename... Args>
  constexpr auto eh_t(Args &&...a) const noexcept {
    // TODO: enforce all EH's are actually exactly the same.
    return this->common_t(::cordo::meta.concat(this->eh1_t((Args &&)a)...));
  }

  template <typename EH, fallible<EH> F>
  constexpr decltype(auto) result1(F &&v) const noexcept {
    return EH::value((F &&)v);
  }
  template <typename T>
  constexpr decltype(auto) result1(T &&v) const noexcept {
    return (T &&)v;
  }
} fallible_helpers{};

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

  template <typename A, typename... Args>
  constexpr auto eh_wrapper(cordo::null_t, cordo::null_t, const algo<A> &a,
                            Args &&...args) const
      CORDO_INTERNAL_ALIAS_(this->resolve(::cordo::overload_prio_t<4>{}, a,
                                          (Args &&)args...));

  template <typename EH, typename E, typename A, typename... Args>
  constexpr auto eh_wrapper(cordo::tag_t<EH>, cordo::tag_t<E>, const algo<A> &a,
                            Args &&...args) const
      noexcept(noexcept(EH::template make_result<E>(
          this->eh_wrapper(cordo::null_t{}, cordo::null_t{}, a,
                           fallible_helpers.result1((Args &&)args)...))))
          -> decltype(EH::template make_result<E>(
              this->eh_wrapper(cordo::null_t{}, cordo::null_t{}, a,
                               fallible_helpers.result1((Args &&)args)...))) {
    auto err = EH::template err_opt<E>();
    bool proceed = ([&err]<typename T>(T &&v) {
      if constexpr (fallible<decltype(v), EH>) {
        if (!EH::success((T &&)v)) {
          err = EH::template err_opt<E>(EH::err((T &&)v));
          return false;
        }
        return true;
      } else {
        return true;
      }
    }(args) && ...);
    return proceed ? EH::template make_result<E>(this->eh_wrapper(
                         cordo::null_t{}, cordo::null_t{}, a,
                         fallible_helpers.result1((Args &&)args)...))
                   : err.move();
  }

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
