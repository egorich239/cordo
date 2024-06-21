#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/algo.hh"
#include "cordo/impl/meta.hh"

namespace cordo_internal_get2 {

struct get2_t final {};
struct get_as_t final {
  template <typename T, ::cordo::accessor A, typename S>
  constexpr auto operator()(::cordo::tag_t<T>, S&& s, A a) const
      -> decltype(::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a))
    requires(std::is_same_v<T, std::remove_cvref_t<typename A::value_t>>)
  {
    // TODO: suboptimal. What I really want to say here is ::cordo::get(s, a)
    return ::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a);
  }
  template <typename T, ::cordo::erased_accessor A, typename S>
  constexpr void operator()(::cordo::tag_t<T>, S&& s, A a) const
    requires(!std::is_same_v<T, std::remove_cvref_t<typename A::value_t>>)
  = delete;
};

struct get_algo_t final {
  template <::cordo::accessor A>
  constexpr auto operator()(const typename A::tuple_t& s, A a) const
      -> decltype(::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a)) {
    return ::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a);
  }
  template <::cordo::accessor A>
  constexpr auto operator()(typename A::tuple_t& s, A a) const
      -> decltype(::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a)) {
    return ::cordo::invoke(::cordo::algo_t<get2_t{}>{}, s, a);
  }
  template <::cordo::accessor A>
  constexpr void operator()(typename A::tuple_t&& s, A a) const = delete;

  template <typename T, ::cordo::erased_accessor A>
  constexpr auto as(const typename A::tuple_t& s, A a) const
      -> decltype(::cordo::invoke(::cordo::algo_t<get_as_t{}>{},
                                  ::cordo::tag_t<T>{}, s, a)) {
    return ::cordo::invoke(::cordo::algo_t<get_as_t{}>{}, ::cordo::tag_t<T>{},
                           s, a);
  }
  template <typename T, ::cordo::erased_accessor A>
  constexpr auto as(typename A::tuple_t& s, A a) const
      -> decltype(::cordo::invoke(::cordo::algo_t<get_as_t{}>{},
                                  ::cordo::tag_t<T>{}, s, a)) {
    return ::cordo::invoke(::cordo::algo_t<get_as_t{}>{}, ::cordo::tag_t<T>{},
                           s, a);
  }
  template <typename T, ::cordo::erased_accessor A>
  constexpr void as(typename A::tuple_t&& s, A a) const = delete;
};

}  // namespace cordo_internal_get2

namespace cordo {
using ::cordo_internal_get2::get2_t;
using ::cordo_internal_get2::get_as_t;

inline constexpr ::cordo_internal_get2::get_algo_t get2{};
}  // namespace cordo
