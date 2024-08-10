#pragma once
#if 0

#include <concepts>
#include <functional>
#include <type_traits>

namespace cordo {
namespace cordo_internal_pipe {

struct error_handler_tag_t final {};
struct fallible2_tag_t final {};

struct bottom_t final {
  bottom_t() = delete;
  constexpr bottom_t(bottom_t&&) noexcept = default;
  constexpr bottom_t(const bottom_t&) noexcept = default;
  constexpr bottom_t& operator=(bottom_t&&) noexcept = default;
  constexpr bottom_t& operator=(const bottom_t&) noexcept = default;
};

template <typename EH>
concept error_handler = requires {
  requires std::is_same_v<typename std::remove_reference_t<EH>::tag_t,
                          error_handler_tag_t>;
};

template <typename T>
concept fallible2 = requires(std::remove_cvref_t<T> v) {
  requires std::is_same_v<typename decltype(v)::tag_t, fallible2_tag_t>;
  requires error_handler<typename decltype(v)::eh_t>;

  typename decltype(v)::value_t;
  typename decltype(v)::error_t;
};

template <typename T>
concept infallible2 =
    fallible2<T> &&
    !std::is_same_v<typename std::remove_cvref_t<T>::value_t, bottom_t> &&
    std::is_same_v<typename std::remove_cvref_t<T>::error_t, bottom_t>;

template <typename T>
concept impossible2 =
    fallible2<T> &&
    std::is_same_v<typename std::remove_cvref_t<T>::value_t, bottom_t> &&
    !std::is_same_v<typename std::remove_cvref_t<T>::error_t, bottom_t>;

template <typename EH>
struct pipe_helper_t final {
  template <fallible2 F>
  constexpr decltype(auto) get_value(F&& f) const
      noexcept(noexcept(EH::get_value((F&&)f)))
    requires(!infallible2<F> &&
             std::is_same_v<EH, typename std::remove_cvref_t<F>::eh_t>)
  {
    return EH::get_value((F&&)f);
  }

  template <infallible2 F>
  constexpr decltype(auto) get_value(F&& f) const
      noexcept(std::is_nothrow_move_constructible_v<F>)
    requires(!fallible2<F>)
  {
    return (F&&)f;
  }

  template <fallible2 F>
  constexpr decltype(auto) get_error(F&& f) const
      noexcept(noexcept(EH::get_error((F&&)f)))
    requires(!infallible2<F> &&
             std::is_same_v<EH, typename std::remove_cvref_t<F>::eh_t>)
  {
    return EH::get_error((F&&)f);
  }

  template <infallible2 F>
  constexpr decltype(auto) get_error(F&& f) const
      noexcept(std::is_nothrow_move_constructible_v<F>)
    requires(!fallible2<F>)
  {
    return (F&&)f;
  }

  template <fallible2 F>
  constexpr decltype(auto) make_value(F&& f) const
      noexcept(std::is_nothrow_move_constructible_v<F>)
    requires(std::is_same_v<EH, typename std::remove_cvref_t<F>::eh_t>)
  {
    return (F&&)f;
  }
  template <infallible2 F>
  constexpr decltype(auto) make_value(F&& f) const
      noexcept(noexcept(EH::template make_value<bottom_t>((F&&)f)))
    requires(!fallible2<F>)
  {
    return EH::template make_value<bottom_t>((F&&)f);
  }

  template <typename E>
  constexpr decltype(auto) make_error(E&& e) const
      noexcept(noexcept(EH::template make_error<bottom_t>((E&&)e))) {
    return EH::template make_error<bottom_t>((E&&)e);
  }

  template <typename E, infallible2 F>
  constexpr decltype(auto) error_cast(F&& f) const
      noexcept(noexcept(EH::template make_value<E>(get_value((F&&)f)))) {
    return EH::template make_value<E>(get_value((F&&)f));
  }

  template <typename E, fallible2 F>
  constexpr decltype(auto) error_cast(F&& f) const
      noexcept(std::is_nothrow_move_constructible_v<F>)
    requires(!infallible2<F> &&
             std::is_same_v<EH, typename std::remove_cvref_t<F>::eh_t> &&
             std::is_same_v<E, typename std::remove_cvref_t<F>::error_t>)
  {
    return (F&&)f;
  }

  template <typename V, impossible2 F>
  constexpr decltype(auto) value_cast(F&& f) const
      noexcept(noexcept(EH::template make_error<V>(get_error((F&&)f)))) {
    return EH::template make_error<V>(get_error((F&&)f));
  }

  template <typename V, typename F>
  constexpr decltype(auto) value_cast(F&& f) const
      noexcept(noexcept(EH::template make_error<V>((F&&)f)))
    requires(!fallible2<F>)
  {
    return EH::template make_error<V>((F&&)f);
  }

  template <fallible2 F>
  constexpr decltype(auto) make_result(F&& f) const
      noexcept(std::is_nothrow_move_constructible_v<F>)
    requires(std::is_same_v<EH, typename std::remove_cvref_t<F>::eh_t>)
  {
    return (F&&)f;
  }

  template <typename F>
  constexpr auto make_result(F&& f) const
      noexcept(std::is_nothrow_move_constructible_v<F>)
          -> decltype(this->make_error((F&&)f))
    requires(!fallible2<F>)
  {
    return make_error((F&&)f);
  }

  template <typename F>
  constexpr auto make_result(F&& f) const
      noexcept(std::is_nothrow_move_constructible_v<F>)
          -> decltype(this->make_value((F&&)f))
    requires(!fallible2<F>)
  {
    return make_value((F&&)f);
  }

  template <typename F, std::invocable<decltype(std::declval<F&&>())> Fn>
  constexpr decltype(auto) then(F&& v,
                                Fn&& fn) noexcept(noexcept(std::invoke((Fn&&)fn,
                                                                       (F&&)v)))
    requires(!fallible2<F>)
  {
    return std::invoke((Fn&&)fn, (F&&)v);
  }

  template <fallible2 F,
            std::invocable<decltype(EH::get_value(std::declval<F&&>()))> Fn>
  constexpr decltype(auto) then(F&& v, Fn&& fn) noexcept(noexcept(
      EH::has_value((F&&)v)
          ? error_cast<typename std::remove_cvref_t<F>::error_t>(
                make_result(std::invoke((Fn&&)fn, EH::get_value((F&&)v))))
          : value_cast<typename decltype(make_result(
                std::invoke((Fn&&)fn, EH::get_value((F&&)v))))::value_t>(
                get_error((F&&)v))))
    requires(std::is_same_v<EH, typename std::remove_cvref_t<F>::eh_t>)
  {
    return EH::has_value((F&&)v)
               ? error_cast<typename std::remove_cvref_t<F>::error_t>(
                     make_result(std::invoke((Fn&&)fn, EH::get_value((F&&)v))))
               : value_cast<typename decltype(make_result(
                     std::invoke((Fn&&)fn, EH::get_value((F&&)v))))::value_t>(
                     get_error((F&&)v));
  }
};

}  // namespace cordo_internal_pipe

using cordo_internal_pipe::bottom_t;
using cordo_internal_pipe::error_handler_tag_t;
using cordo_internal_pipe::fallible2_tag_t;
}  // namespace cordo

#endif