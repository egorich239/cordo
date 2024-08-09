#pragma once

#include <cstdint>
#include <exception>
#include <type_traits>
#include <variant>

#if !defined(__GNUC__) || defined(__EXCEPTIONS)
#include <stdexcept>
#endif

#include "cordo/impl/core/algo.hh"

namespace mirror_internal_result {

enum class mirror_error : uint32_t {
  UNKNOWN = 0,
  INVALID_UNWRAP = 1,
};

template <typename EH, typename T>
class mirror_result final {
  static_assert(!std::is_same_v<T, mirror_error>);

 public:
  using tag_t = cordo::fallible_tag;
  using eh_t = EH;
  using result_t = T;
  using error_t = mirror_error;

  template <typename U>
  constexpr mirror_result(U&& v) noexcept(
      std::is_nothrow_constructible_v<T, U&&>)
    requires(std::is_constructible_v<T, U &&>)
      : state_{T{(U&&)v}} {}
  constexpr mirror_result(mirror_error e) noexcept : state_{e} {}

  constexpr mirror_result(mirror_result&&) = default;
  constexpr mirror_result(const mirror_result&) = default;
  constexpr mirror_result& operator=(mirror_result&&) = default;
  constexpr mirror_result& operator=(const mirror_result&) = default;

  constexpr bool ok() const noexcept { return state_.index() == 0; }

  constexpr const T& value() const& noexcept { return std::get<0>(state_); }
  constexpr T& value() & noexcept { return std::get<0>(state_); }
  constexpr T&& value() && noexcept { return std::get<0>(state_); }

  constexpr mirror_error error() const noexcept { return std::get<1>(state_); }

 private:
  std::variant<T, mirror_error> state_;
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
  static constexpr bool has_result(T&&) noexcept {
    return true;
  }
  template <typename T>
  static constexpr decltype(auto) as_result(T&& v) noexcept {
    return (T&&)v;
  }
  template <typename T>
  static constexpr auto make_result(T&& v,
                                    cordo::tag_t<mirror_error>) noexcept {
    return (T&&)v;
  }
  static constexpr eh_terminator_t make_error(mirror_error err) noexcept {
    return H{};
  }

  template <typename T>
  static constexpr decltype(auto) as_error(T&&) noexcept = delete;
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

struct eh_result final {
  template <typename T>
  static constexpr bool has_result(mirror_result<eh_result, T>&& v) noexcept {
    return v.ok();
  }
  template <typename T>
  static constexpr decltype(auto) as_result(
      mirror_result<eh_result, T>&& v) noexcept {
    return std::move(v).value();
  }
  template <typename T>
  static constexpr decltype(auto) as_error(
      mirror_result<eh_result, T>&& v) noexcept {
    return v.error();
  }

  template <typename T>
  static constexpr auto make_result(mirror_result<eh_result, T>&& v,
                                    cordo::tag_t<mirror_error>) noexcept {
    return (T&&)v;
  }
  template <typename T>
  static constexpr auto make_result(T&& v,
                                    cordo::tag_t<mirror_error>) noexcept {
    return mirror_result<mirror_error, T>{(T&&)v};
  }
  static constexpr mirror_error make_error(mirror_error err) noexcept {
    return err;
  }
};

}  // namespace mirror_internal_result