#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_cpo_core {
struct fallible_tag final {};

template <typename T>
concept fallible = requires(std::remove_cvref_t<T> v) {
  requires std::is_same_v<typename decltype(v)::tag_t, fallible_tag>;

  typename decltype(v)::eh_t;
  typename decltype(v)::result_t;
  typename decltype(v)::error_t;
};

template <typename Fn>
struct pipe_t final {
 public:
  constexpr explicit pipe_t(Fn &&fn) noexcept(
      std::is_nothrow_move_constructible_v<Fn>)
      : fn_{(Fn &&)fn} {}

  template <typename..., fallible V,
            typename EH = typename std::remove_cvref_t<V>::eh_t>
  friend constexpr decltype(auto) operator|(V &&v, pipe_t &&self)
      CORDO_INTERNAL_RETURN_(
          EH::has_result((V &&)v)
              ? EH::make_result(
                    std::invoke(((pipe_t &&)self).fn_, EH::as_result((V &&)v)),
                    cordo::tag_t<
                        std::remove_cvref_t<decltype(EH::as_error((V &&)v))>>{})
              : EH::as_error((V &&)v));

  template <typename..., typename V>
  friend constexpr decltype(auto) operator|(V &&v, pipe_t &&self) noexcept(
      noexcept(std::invoke(((pipe_t &&)self).fn_, (V &&)v)))
    requires(!fallible<std::remove_cvref_t<V>>)
  {
    return std::invoke(self.fn_, (V &&)v);
  }

 private:
  Fn fn_;
};

struct piped_t final {};

template <typename A>
struct algo final {
  static_assert(((void)A{}, true),
                "algorithm traits must be constexpr-constructible");

 private:
  template <typename... Args>
  constexpr auto invoke(::cordo::overload_prio_t<3>, Args &&...args) const
      CORDO_INTERNAL_ALIAS_(customize(*this, (Args &&)args...));
  template <typename... Args, typename A2 = A>
  constexpr auto invoke(::cordo::overload_prio_t<2>, Args &&...args) const
      CORDO_INTERNAL_ALIAS_(customize(*this, typename A2::adl_tag{},
                                      (Args &&)args...));
  template <typename... Args>
  constexpr auto invoke(::cordo::overload_prio_t<1>, Args &&...args) const
      CORDO_INTERNAL_ALIAS_(A{}(*this, (Args &&)args...));

 public:
  template <typename... Args>
  constexpr decltype(auto) operator()(piped_t, Args &&...args) const
      CORDO_INTERNAL_RETURN_(pipe_t{
          [self = *this, ... args = (Args &&)args](auto &&v) {
            return self((decltype(v) &&)v, (Args &&)args...);
          }});

  template <typename... Args>
  constexpr decltype(auto) operator()(Args &&...args) const  //
      CORDO_INTERNAL_RETURN_(this->invoke(::cordo::overload_prio_t<3>{},
                                          (Args &&)args...));

  // TODO: this stop-gap provides some minimum reasonable error-description,
  // and prevents the 1000s lines of gibberish, but maybe we could improve
  // the informativeness of it all?
  constexpr auto operator()(...) const = delete;
};

}  // namespace cordo_internal_cpo_core

namespace cordo {
using ::cordo_internal_cpo_core::algo;
using ::cordo_internal_cpo_core::fallible;
using ::cordo_internal_cpo_core::fallible_tag;
inline constexpr ::cordo_internal_cpo_core::piped_t piped{};
}  // namespace cordo

namespace cordo_internal_cpo {
struct adl_tag final {};
}  // namespace cordo_internal_cpo
