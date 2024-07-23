#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/core/reflect.hh"

namespace cordo_internal_mirror {

template <typename Traits>
concept mirror_traits = requires {
  typename Traits::t;
  requires !std::is_reference_v<typename Traits::t>;
};

template <typename T>
struct mirror_unsupported final {
  using t = T;
};

struct mirror_traits_ctor_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;

  // The idea of unsupported type is to simplify codegens for structs:
  // every field will get a trait, worst case it will be unsupported.
  // This however still allows us to later filter them with cordo::skip().
  template <typename T>
  constexpr auto operator()(const ::cordo::algo<mirror_traits_ctor_t>&,
                            ::cordo::tag_t<T>) const noexcept {
    return mirror_unsupported<T>{};
  }
};

struct mirror_traits_name_t final {
 private:
  template <typename T>
  constexpr auto resolve(::cordo::overload_prio_t<1>, T) const
      CORDO_INTERNAL_ALIAS_(typename T::name{});

  template <typename T>
  constexpr auto resolve(::cordo::overload_prio_t<0>, T) const noexcept {
    return ::cordo::null_t{};
  }

 public:
  using adl_tag = ::cordo_internal_cpo::adl_tag;

  template <typename T>
  constexpr auto operator()(const ::cordo::algo<mirror_traits_name_t>&,
                            T v) const noexcept {
    return this->resolve(::cordo::overload_prio_t<1>{}, v);
  }
};

struct mirror_traits_subscript_keys_t final {
 private:
  template <typename T>
  constexpr auto resolve(::cordo::overload_prio_t<1>, T) const
      CORDO_INTERNAL_ALIAS_(typename T::subscript_keys{});

  template <typename T>
  constexpr auto resolve(::cordo::overload_prio_t<0>, T) const noexcept {
    return ::cordo::null_t{};
  }

 public:
  using adl_tag = ::cordo_internal_cpo::adl_tag;

  template <typename T>
  constexpr auto operator()(
      const ::cordo::algo<mirror_traits_subscript_keys_t>&, T v) const
      CORDO_INTERNAL_ALIAS_(this->resolve(::cordo::overload_prio_t<1>{}, v));
};

struct mirror_assign_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;

  template <mirror_traits Traits, typename T, typename U>
  constexpr decltype(auto) operator()(const ::cordo::algo<mirror_assign_t>&,
                                      Traits, T& target, U&& value) const
      CORDO_INTERNAL_RETURN_(target = (U&&)value);

  template <mirror_traits Traits, typename T, typename U>
  void operator()(const ::cordo::algo<mirror_assign_t>&, Traits, T&& target,
                  U&& value) const = delete;
};

struct mirror_subscript_key_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;
};

struct mirror_unwrap_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;
};

inline constexpr ::cordo::algo<mirror_traits_ctor_t> mirror_traits_ctor;
inline constexpr ::cordo::algo<mirror_traits_name_t> mirror_traits_name;
inline constexpr ::cordo::algo<mirror_traits_subscript_keys_t>
    mirror_traits_subscript_keys;

inline constexpr ::cordo::algo<mirror_assign_t> mirror_assign;
inline constexpr ::cordo::algo<mirror_subscript_key_t> mirror_subscript_key;
inline constexpr ::cordo::algo<mirror_unwrap_t> mirror_unwrap;

template <mirror_traits Traits, typename M>
struct mirror_ref final {
  using traits = Traits;

  M* mirror;

  constexpr M& operator*() const noexcept { return *mirror; }
  constexpr M* operator->() const noexcept { return mirror; }

  constexpr decltype(auto) rep() const noexcept { return mirror->value_; }
};

template <mirror_traits Traits>
class mirror_api final {
  using T = typename Traits::t;
  using rep = typename Traits::rep;
  rep value_;

  constexpr auto ref() const noexcept {
    return mirror_ref<Traits, const mirror_api>{this};
  }
  constexpr auto ref() noexcept { return mirror_ref<Traits, mirror_api>{this}; }

  template <auto K, typename R>
  static constexpr decltype(auto) subscript(::cordo::overload_prio_t<1>, R ref,
                                            ::cordo::key_t<K> k)
      CORDO_INTERNAL_RETURN_(mirror_subscript_key(ref, k));

  // TODO: inspirational goal: subscript(non-key-index)

 public:
  using traits = Traits;

  explicit constexpr mirror_api(rep&& value, Traits) noexcept
      : value_{(rep&&)value} {}

  constexpr const T& v() const noexcept
    requires(std::is_same_v<T&, rep>)
  {
    return value_;
  }
  constexpr T& v() noexcept
    requires(std::is_same_v<T&, rep>)
  {
    return value_;
  }

  template <typename..., typename R = mirror_ref<Traits, mirror_api>,
            typename = decltype(mirror_unwrap(std::declval<R>()))>
  constexpr auto unwrap() noexcept(noexcept(mirror_unwrap(this->ref()))) {
    return mirror_unwrap(this->ref());
  }
  template <typename..., typename R = mirror_ref<Traits, const mirror_api>,
            typename = decltype(mirror_unwrap(std::declval<R>()))>
  constexpr auto unwrap() const noexcept(noexcept(mirror_unwrap(this->ref()))) {
    return mirror_unwrap(this->ref());
  }

  template <typename U>
  constexpr auto operator=(U&& v) CORDO_INTERNAL_ALIAS_(
      ((void)mirror_assign(Traits{}, (rep&&)this->value_, (U&&)v)), *this);

  template <typename K>
  constexpr auto operator[](K&& k) const
      CORDO_INTERNAL_ALIAS_(mirror_api::subscript(::cordo::overload_prio_t<1>{},
                                                  this->ref(), (K&&)k));

  template <typename K>
  constexpr auto operator[](K&& k)
      CORDO_INTERNAL_ALIAS_(mirror_api::subscript(::cordo::overload_prio_t<1>{},
                                                  this->ref(), (K&&)k));

 private:
  friend class mirror_ref<traits, mirror_api>;
  friend class mirror_ref<traits, const mirror_api>;
};
template <typename Traits>
mirror_api(typename Traits::rep&&, Traits) -> mirror_api<Traits>;

struct mirror_fn final {
  template <typename T>
  constexpr auto t(::cordo::tag_t<T>) const
      CORDO_INTERNAL_RETURN_(mirror_traits_ctor(::cordo::tag_t<T>{}));

  template <typename T>
  constexpr auto traits(T&&) const
      CORDO_INTERNAL_RETURN_(this->t(::cordo::tag_t<T>{}));

  template <typename T>
  constexpr auto operator()(T&& v) const
      CORDO_INTERNAL_RETURN_(mirror_api((T&&)v, this->traits((T&&)v)));
};
}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirror_api;
using ::cordo_internal_mirror::mirror_ref;

using ::cordo_internal_mirror::mirror_traits_ctor;
using ::cordo_internal_mirror::mirror_traits_name;
using ::cordo_internal_mirror::mirror_traits_subscript_keys;

using ::cordo_internal_mirror::mirror_assign;
using ::cordo_internal_mirror::mirror_subscript_key;
using ::cordo_internal_mirror::mirror_unwrap;

inline constexpr ::cordo_internal_mirror::mirror_fn mirror{};
}  // namespace cordo