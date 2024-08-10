#pragma once

#include <cstdint>
#include <exception>
#include <type_traits>
#include <variant>

#if !defined(__GNUC__) || defined(__EXCEPTIONS)
#include <stdexcept>
#endif

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/pipe.hh"

namespace mirror_internal_result {

enum class mirror_error : uint32_t {
  UNKNOWN = 0,
  INVALID_UNWRAP = 1,
};

struct eh_terminator_t final {
  template <typename T>
  [[noreturn]] operator T() {
    // Hasta la vista!
    std::terminate();
  }
};

template <typename H>
struct eh_stack_unwind final {
  template <typename T>
  static constexpr auto make_result(T&& v,
                                    cordo::tag_t<mirror_error>) noexcept {
    return (T&&)v;
  }
  template <typename T>
  static constexpr auto make_result(T&& v) noexcept {
    return make_result((T&&)v, {});
  }
  static constexpr H make_error(mirror_error err) noexcept { return H{}; }
};

using eh_terminate = eh_stack_unwind<eh_terminator_t>;

#if defined(__GNUC__) && !defined(__EXCEPTIONS)
using eh_throw = eh_terminate;
#else

struct eh_exception_t final {
  template <typename T>
  operator T() {
    // TODO: better details.
    throw std::runtime_error("something went wrong");
  }
};
using eh_throw = eh_stack_unwind<eh_exception_t>;
#endif

template <typename T>
class mirror_result final {
  static_assert(!std::is_same_v<T, mirror_error>);

 public:
  template <typename U>
  constexpr mirror_result(U&& v) noexcept(
      std::is_nothrow_constructible_v<T, U&&>)
    requires(std::is_constructible_v<T, U &&>)
      : state_{T{(U&&)v}} {}
  constexpr mirror_result(T&& v) noexcept(
      std::is_nothrow_move_constructible_v<T>)
      : state_{(T&&)v} {}
  constexpr mirror_result(mirror_error e) noexcept : state_{e} {}

  constexpr mirror_result(mirror_result&&) = default;
  constexpr mirror_result(const mirror_result&) = default;
  constexpr mirror_result& operator=(mirror_result&&) = default;
  constexpr mirror_result& operator=(const mirror_result&) = default;

  constexpr bool ok() const noexcept { return state_.index() == 0; }

  constexpr const T& value() const& noexcept { return std::get<0>(state_); }
  constexpr T& value() & noexcept { return std::get<0>(state_); }
  constexpr T&& value() && noexcept { return std::get<0>(std::move(state_)); }

  constexpr mirror_error error() const noexcept { return std::get<1>(state_); }

 private:
  std::variant<T, mirror_error> state_;
};

struct eh_result final {
  template <typename T>
  static constexpr auto make_result(mirror_result<T>&& v,
                                    cordo::tag_t<mirror_error>) noexcept {
    return (T&&)v;
  }
  template <typename T>
  static constexpr auto make_result(T&& v,
                                    cordo::tag_t<mirror_error>) noexcept {
    return mirror_result<T>{(T&&)v};
  }
  template <typename T>
  static constexpr auto make_result(T&& v) noexcept {
    return make_result((T&&)v, {});
  }
  static constexpr mirror_error make_error(mirror_error err) noexcept {
    return err;
  }
};

template <typename T>
constexpr auto customize(decltype(cordo::fallible_get_factory),
                         const mirror_result<T>& r) noexcept {
  return eh_result{};
}

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_has_value),
                                   const mirror_result<T>& r) noexcept {
  return r.ok();
}

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_value),
                                   const mirror_result<T>& r) noexcept {
  return r.value();
}
template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_value),
                                   mirror_result<T>& r) noexcept {
  return r.value();
}
template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_value),
                                   mirror_result<T>&& r) noexcept {
  return std::move(r).value();
}

template <typename T>
constexpr decltype(auto) customize(decltype(cordo::fallible_get_error),
                                   const mirror_result<T>& r) noexcept {
  return r.error();
}

static_assert(cordo::fallible<mirror_result<int>>);

}  // namespace mirror_internal_result

namespace cordo {
using mirror_internal_result::eh_result;
using mirror_internal_result::eh_terminate;
using mirror_internal_result::mirror_error;
using mirror_internal_result::mirror_result;
}  // namespace cordo