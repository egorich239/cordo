#pragma once

#include <cstddef>
#include <type_traits>

namespace cordo_internal_meta {

struct null_t final {};

template <typename T>
struct tag_t final {
  using type = T;
};

template <typename T>
struct base_tag_t {};

template <auto V>
struct value_t final {
  constexpr auto operator()() const noexcept { return V; }
};

template <typename... T>
struct types_t final {};

template <auto... V>
struct values_t final {};

template <std::size_t N>
struct overload_prio_t : overload_prio_t<N - 1> {};
template <>
struct overload_prio_t<0> {};

template <typename... Vs>
struct is_unique_impl_marker_t;

template <>
struct is_unique_impl_marker_t<> {};

template <typename V>
struct is_unique_impl_marker_t<V> : base_tag_t<V> {};

template <typename V, typename... Vs>
struct is_unique_impl_marker_t<V, Vs...> : base_tag_t<V>,
                                           is_unique_impl_marker_t<Vs...> {};

class is_unique_impl final {
  template <typename... Vs>
  struct I {
    static constexpr char resolve(base_tag_t<Vs>...) noexcept { return {}; }
  };

 public:
  constexpr is_unique_impl() noexcept = default;
  template <typename... Vs,
            typename = decltype(I<Vs...>::resolve(
                (base_tag_t<Vs>{}, is_unique_impl_marker_t<Vs...>{})...))>
  constexpr bool operator()(overload_prio_t<1>, tag_t<Vs>...) const noexcept {
    return true;
  }
  constexpr bool operator()(overload_prio_t<0>, ...) const noexcept {
    return false;
  }
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

struct meta_t final {
  template <auto... V>
  constexpr auto lift_t(values_t<V...>) const noexcept {
    return types_t<value_t<V>...>{};
  }

  template <auto... V>
  constexpr auto unlift_v(types_t<value_t<V>...>) const noexcept {
    return values_t<V...>{};
  }

  constexpr auto lift_tag(tag_t<null_t>) const noexcept { return types_t<>{}; }
  template <typename T>
  constexpr auto lift_tag(tag_t<T>) const noexcept {
    return types_t<T>{};
  }

  template <typename... T>
  constexpr auto concat(types_t<T...> v) const noexcept {
    return v;
  }
  template <typename... T, typename... U>
  constexpr auto concat(types_t<T...>, types_t<U...>) const noexcept {
    return types_t<T..., U...>{};
  }

  template <typename... T, typename... U, typename... Ts>
  constexpr auto concat(types_t<T...> v, types_t<U...> u,
                        Ts... ts) const noexcept {
    return concat(concat(v, u), ts...);
  }

  template <auto... T>
  constexpr auto concat(values_t<T...> v) const noexcept {
    return v;
  }
  template <auto... T, auto... U>
  constexpr auto concat(values_t<T...>, values_t<U...>) const noexcept {
    return values_t<T..., U...>{};
  }

  template <auto... T, auto... U, typename... Ts>
  constexpr auto concat(values_t<T...> v, values_t<U...> u,
                        Ts... ts) const noexcept {
    return concat(concat(v, u), ts...);
  }

  template <typename... T>
  constexpr auto filter(types_t<T...>) const noexcept {
    return concat(lift_tag(tag_t<T>{})...);
  }

  template <auto... T, typename Fn>
  constexpr auto filter(values_t<T...>, Fn) const noexcept {
    return unlift_v(
        filter(types_t<std::conditional_t<Fn{}(T), value_t<T>, null_t>...>{}));
  }

  template <typename... T>
  constexpr auto unique(types_t<T...>) const noexcept {
    return is_unique_impl{}(overload_prio_t<1>{}, tag_t<T>{}...);
  }
};

template <typename T>
struct typeid_t final {
  static constexpr char key = 0;
};

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
using ::cordo_internal_meta::null_t;
using ::cordo_internal_meta::overload_prio_t;
using ::cordo_internal_meta::same_constness_as_t;
using ::cordo_internal_meta::tag_t;
using ::cordo_internal_meta::typeid_t;
using ::cordo_internal_meta::types_t;
using ::cordo_internal_meta::value_t;
using ::cordo_internal_meta::values_t;
inline constexpr ::cordo_internal_meta::make_li_t make_li{};
inline constexpr ::cordo_internal_meta::li_push_t li_push{};
inline constexpr ::cordo_internal_meta::meta_t meta{};
}  // namespace cordo