#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/mirror.hh"

namespace cordo_internal_option {

struct option_get_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  template <typename T>
  constexpr const T* operator()(const std::optional<T>& o) const noexcept {
    return o ? &o.value() : nullptr;
  }
  template <typename T>
  constexpr T* operator()(std::optional<T>& o) const noexcept {
    return o ? &o.value() : nullptr;
  }
  template <typename T>
  constexpr const T* operator()(const std::unique_ptr<T>& o) const noexcept {
    return o.get();
  }
  template <typename T>
  constexpr T* operator()(std::unique_ptr<T>& o) const noexcept {
    return o.get();
  }
};
using option_get_cpo = ::cordo::cpo_t<option_get_cpo_t{}>;

struct option_get_t final {
  template <typename T>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(),          //
      (T& v) const,        //
      (::cordo::invoke(option_get_cpo{}, v)));
};

struct option_set_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  template <typename T, typename U>
  constexpr void operator()(std::optional<T>& o, U&& v) const noexcept
    requires(std::is_assignable_v<std::optional<T>, U &&>)
  {
    o = (U&&)v;
  }
  template <typename T, typename U>
  constexpr void operator()(std::unique_ptr<T>& o, U&& v) const noexcept
    requires(std::is_assignable_v<std::unique_ptr<T>,
                                  decltype(std::make_unique<T>((U &&) v))>)
  {
    return o = std::make_unique<T>((U&&)v);
  }
};
using option_set_cpo = ::cordo::cpo_t<option_set_cpo_t{}>;

struct option_set_t final {
  template <typename T, typename U>
  CORDO_INTERNAL_LAMBDA_(   //
      operator(),           //
      (T& o, U&& v) const,  //
      (::cordo::invoke(option_set_cpo{}, o, (U&&)v)));
};

}  // namespace cordo_internal_option

namespace cordo {
using ::cordo_internal_option::option_get_cpo;
using ::cordo_internal_option::option_set_cpo;
inline constexpr ::cordo_internal_option::option_get_t option_get{};
inline constexpr ::cordo_internal_option::option_set_t option_set{};
}  // namespace cordo

namespace cordo_internal_option {

template <typename T>
concept option_type =
    std::is_pointer_v<decltype(::cordo::option_get(std::declval<T>()))>;

template <option_type T>
struct option_ final {
  static_assert(::cordo::mirrored<inner_t>);

  using tuple_t = T;
  using inner_t =
      std::remove_pointer_t<decltype(::cordo::option_get(std::declval<T>()))>;
};

template <option_meta M, option_type S>
class option_mirror final {
  S& object_;

 public:
  constexpr explicit option_mirror(S& object) noexcept : object_{object} {}

  template <typename U>
  CORDO_INTERNAL_LAMBDA_(  //
      operator=,           //
      (U&& v),             //
      (::cordo::option_set(object_, (U&&)v)));
};

}  // namespace cordo_internal_option