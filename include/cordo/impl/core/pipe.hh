#pragma once

#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "cordo/impl/core/invoke.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo {
namespace cordo_pipe_extensions {
struct adl_tag final {};
}  // namespace cordo_pipe_extensions
namespace cordo_internal_pipe {

struct fallible_get_factory_cpo final {};
inline constexpr cpo_t<fallible_get_factory_cpo{},
                       cordo_pipe_extensions::adl_tag>
    fallible_get_factory{};

struct fallible_has_value_cpo final {};
inline constexpr cpo_t<fallible_has_value_cpo{}, cordo_pipe_extensions::adl_tag>
    fallible_has_value{};

struct fallible_get_value_cpo final {};
inline constexpr cpo_t<fallible_get_value_cpo{}, cordo_pipe_extensions::adl_tag>
    fallible_get_value{};

struct fallible_get_error_cpo final {};
inline constexpr cpo_t<fallible_get_error_cpo{}, cordo_pipe_extensions::adl_tag>
    fallible_get_error{};

template <typename T>
concept fallible = requires(T&& v) {
  cordo::invoke(fallible_get_factory, (T&&)v);
  { cordo::invoke(fallible_has_value, (T&&)v) } -> std::same_as<bool>;
  cordo::invoke(fallible_get_value, (T&&)v);
  cordo::invoke(fallible_get_error, (T&&)v);
};

template <typename T>
concept not_fallible = !fallible<T>;

struct pipe_then_fn final {
  template <typename Fn, typename... Args>
  constexpr decltype(auto) void_guard(Fn&& fn, Args&&... args) const
      noexcept(noexcept(std::invoke((Fn&&)fn, (Args&&)args...)))
    requires(!std::is_same_v<std::invoke_result_t<Fn &&, Args && ...>, void>)
  {
    return std::invoke((Fn&&)fn, (Args&&)args...);
  }

  template <typename Fn, typename... Args>
  constexpr unit_t void_guard(Fn&& fn, Args&&... args) const
      noexcept(noexcept(std::invoke((Fn&&)fn, (Args&&)args...)))
    requires(std::is_same_v<std::invoke_result_t<Fn &&, Args && ...>, void>)
  {
    std::invoke((Fn&&)fn, (Args&&)args...);
    return unit;
  }

  template <typename... Args, not_fallible F, std::invocable<F&&, Args&&...> Fn>
  constexpr decltype(auto) operator()(F&& v, Fn&& fn, Args&&... args) const
      CORDO_INTERNAL_RETURN_(this->void_guard((Fn&&)fn, (F&&)v,
                                              (Args&&)args...));

  template <typename... Args, fallible F,
            std::invocable<decltype(cordo::invoke(fallible_get_value,
                                                  std::declval<F&&>())),
                           Args&&...>
                Fn>
  constexpr decltype(auto) operator()(F&& v, Fn&& fn, Args&&... args) const
      CORDO_INTERNAL_RETURN_(
          cordo::invoke(fallible_has_value, (F&&)v)
              ? cordo::invoke(fallible_get_factory, (F&&)v)
                    .make_result(
                        this->void_guard(
                            (Fn&&)fn, cordo::invoke(fallible_get_value, (F&&)v),
                            (Args&&)args...),
                        cordo::tag_t<decltype(cordo::invoke(fallible_get_error,
                                                            (F&&)v))>{})
              : cordo::invoke(fallible_get_factory, (F&&)v)
                    .make_error(cordo::invoke(fallible_get_error, (F&&)v)));
};
inline constexpr pipe_then_fn pipe_then{};

template <typename Fn, typename... Args>
struct pipe_continuation_t final {
  Fn fn;
  std::tuple<Args...> args;

 private:
  template <typename F, size_t... Idx>
      constexpr decltype(auto) invoke(F&& v, std::index_sequence<Idx...>) &&
      CORDO_INTERNAL_RETURN_(
          pipe_then((F&&)v, std::move(*this).fn,
                    std::get<Idx>(std::move(*this).args)...));

 public:
  template <typename F>
      constexpr decltype(auto) operator()(F&& v) &&
      CORDO_INTERNAL_RETURN_(std::move(*this).invoke(
          (F&&)v, std::make_index_sequence<sizeof...(Args)>{}));

  template <typename P>
  friend decltype(auto) operator|(P&& v, pipe_continuation_t&& self)
      CORDO_INTERNAL_RETURN_(std::move(self)((P&&)v));
};

struct piped_fn final {
  template <typename Fn, typename... Args>
  constexpr decltype(auto) operator()(Fn&& fn, Args&&... args) const
      CORDO_INTERNAL_RETURN_(pipe_continuation_t<Fn, Args...>{
          .fn = (Fn&&)fn,
          .args = {(Args&&)args...},
      });
};
inline constexpr piped_fn piped;

}  // namespace cordo_internal_pipe

using cordo_internal_pipe::fallible;
using cordo_internal_pipe::fallible_get_error;
using cordo_internal_pipe::fallible_get_factory;
using cordo_internal_pipe::fallible_get_value;
using cordo_internal_pipe::fallible_has_value;
using cordo_internal_pipe::piped;
}  // namespace cordo
