#pragma once

#include <cstddef>
#include <type_traits>

namespace cordo_internal_meta {

template <typename T>
struct tag_t final {
  using type = T;
};

template <typename... Vs>
struct li_t;

template <typename V, typename... Vs>
struct li_t<V, Vs...> final {
  constexpr auto size() const noexcept { return 1 + sizeof...(Vs); }

  template <typename I, typename Fn>
  constexpr auto rfold(I&& i, Fn&& f) const {
    return f(t.rfold((I&&)i, (Fn&&)f), h);
  }
  V h;
  li_t<Vs...> t;
};

template <>
struct li_t<> final {
  constexpr std::size_t size() const noexcept { return 0; }

  template <typename I, typename Fn>
  constexpr auto rfold(I&& i, Fn&&) const {
    return i;
  }
};

template <typename... T>
struct types_t final {};

template <auto... V>
struct values_t final {};

template <typename T>
struct typeid_t final {
  static constexpr char key = 0;
};

template <std::size_t N>
struct overload_prio_t : overload_prio_t<N - 1> {};
template <>
struct overload_prio_t<0> {};

template <typename S, typename T>
struct same_constness_as final {
  static_assert(!std::is_reference_v<S> && !std::is_reference_v<T>);

 private:
  template <typename U>
  static constexpr tag_t<const T> impl(tag_t<const U>) noexcept {
    return {};
  }
  template <typename U>
  static constexpr tag_t<T> impl(tag_t<U>) noexcept {
    return {};
  }

 public:
  using type = typename decltype(same_constness_as::impl(tag_t<S>{}))::type;
};

template <typename S, typename T>
using same_constness_as_t = typename same_constness_as<S, T>::type;

struct make_li_t final {
  template <typename... Vs>
  constexpr auto operator()(li_t<Vs...> v) const noexcept {
    return v;
  }

  template <typename V>
  constexpr auto operator()(V v) const noexcept {
    return li_t<V>{v, {}};
  }
};

struct li_push_t final {
  template <typename V, typename... Vs>
  constexpr auto operator()(V t, li_t<Vs...> h) const noexcept {
    return li_t<V, Vs...>{t, h};
  }
};

}  // namespace cordo_internal_meta

namespace cordo {
using ::cordo_internal_meta::li_t;
using ::cordo_internal_meta::overload_prio_t;
using ::cordo_internal_meta::same_constness_as_t;
using ::cordo_internal_meta::tag_t;
using ::cordo_internal_meta::typeid_t;
using ::cordo_internal_meta::types_t;
using ::cordo_internal_meta::values_t;
inline constexpr ::cordo_internal_meta::make_li_t make_li{};
inline constexpr ::cordo_internal_meta::li_push_t li_push{};
}  // namespace cordo