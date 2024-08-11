#pragma once

#include <type_traits>

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/mirror_result.hh"

namespace cordo {
namespace cordo_internal_mirror {

template <typename Traits>
concept mirror_traits = requires {
  requires std::is_trivial_v<Traits>;
  requires std::is_standard_layout_v<Traits>;

  typename Traits::t;
  requires !std::is_reference_v<typename Traits::t>;
};

template <mirror_traits Traits, typename EH>
struct mirror_impl_t final {
  using rep_t = typename Traits::rep;

  static constexpr auto traits() noexcept { return Traits{}; }
  static constexpr auto eh() noexcept { return EH{}; }

  rep_t value;
};

struct mirror_impl_apply_fn final {
  template <typename..., mirror_traits Traits, typename EH, cordo::algo Algo,
            typename... Args>
  constexpr decltype(auto) operator()(mirror_impl_t<Traits, EH>& core,
                                      Algo algo, Args&&... args) const
      CORDO_INTERNAL_RETURN_(algo(Traits{}, core, (Args&&)args...));

  template <typename..., mirror_traits Traits, typename EH, cordo::algo Algo,
            typename... Args>
  constexpr decltype(auto) operator()(mirror_impl_t<Traits, EH>&& core,
                                      Algo algo, Args&&... args) const
      CORDO_INTERNAL_RETURN_(algo(Traits{}, std::move(core), (Args&&)args...));

  template <typename..., mirror_traits Traits, typename EH, cordo::algo Algo,
            typename... Args>
  constexpr decltype(auto) operator()(const mirror_impl_t<Traits, EH>& core,
                                      Algo algo, Args&&... args) const
      CORDO_INTERNAL_RETURN_(algo(Traits{}, core, (Args&&)args...));
};
inline constexpr mirror_impl_apply_fn mirror_impl_apply{};

template <typename T>
struct mirror_unsupported final {
  using t = T;
};

struct mirror_traits_of_const_core_t final {};
inline constexpr ::cordo::algo_t<mirror_traits_of_const_core_t{}>
    mirror_traits_of_const;

struct mirror_traits_ctor_core_t final {
  // The idea of unsupported type is to simplify codegens for structs:
  // every field will get a trait, worst case it will be unsupported.
  // This however still allows us to later filter them with cordo::skip().
  template <typename T>
  constexpr auto operator()(::cordo::tag_t<T>) const noexcept {
    return mirror_unsupported<T>{};
  }

  template <typename T, algo Algo>
  constexpr auto operator()(const Algo& rec, ::cordo::tag_t<T&>) const
      CORDO_INTERNAL_ALIAS_(rec(::cordo::tag_t<T>{}));

  template <typename T, algo Algo>
  constexpr auto operator()(const Algo& rec, ::cordo::tag_t<const T>) const
      CORDO_INTERNAL_ALIAS_(mirror_traits_of_const(rec(::cordo::tag_t<T>{})));

  // TODO: mirror_inlined?
  template <typename T>
  constexpr auto operator()(::cordo::tag_t<T&&>) const noexcept = delete;
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
  template <typename T>
  constexpr auto operator()(T v) const noexcept {
    return this->resolve(::cordo::overload_prio_t<1>{}, v);
  }
};

struct mirror_traits_subscript_keys_core_t final {
 private:
  template <typename T>
  constexpr auto resolve(::cordo::overload_prio_t<1>, T) const
      CORDO_INTERNAL_ALIAS_(typename T::subscript_keys{});

  template <typename T>
  constexpr auto resolve(::cordo::overload_prio_t<0>, T) const noexcept {
    return ::cordo::null_t{};
  }

 public:
  template <typename T>
  constexpr auto operator()(T v) const
      CORDO_INTERNAL_ALIAS_(this->resolve(::cordo::overload_prio_t<1>{}, v));
};

struct mirror_assign_t final {
  template <mirror_traits Traits, typename U>
  constexpr decltype(auto) operator()(Traits, auto& core, U&& value) const
      CORDO_INTERNAL_RETURN_(core.value = (U&&)value);
};

struct mirror_subscript_key_core_t final {};

struct mirror_has_value_core_t final {};
struct mirror_unwrap_core_t final {};

inline constexpr ::cordo::algo_t<mirror_traits_ctor_core_t{}>
    mirror_traits_ctor;
inline constexpr ::cordo::algo_t<mirror_traits_name_t{}> mirror_traits_name;
inline constexpr ::cordo::algo_t<mirror_traits_subscript_keys_core_t{}>
    mirror_traits_subscript_keys;

inline constexpr ::cordo::algo_t<mirror_assign_t{}> mirror_assign;
inline constexpr ::cordo::algo_t<mirror_subscript_key_core_t{}>
    mirror_subscript_key;

inline constexpr ::cordo::algo_t<mirror_has_value_core_t{}> mirror_has_value;
inline constexpr ::cordo::algo_t<mirror_unwrap_core_t{}> mirror_unwrap;

struct make_mirror_impl_fn final {
  template <typename..., mirror_traits Traits, typename EH>
  constexpr auto operator()(EH, Traits, typename Traits::rep&& value) const
      noexcept(std::is_nothrow_move_constructible_v<typename Traits::rep&&>) {
    return mirror_impl_t<Traits, EH>{(typename Traits::rep&&)value};
  }

  template <typename..., mirror_traits Tr0, mirror_traits Traits, typename EH>
  constexpr auto operator()(const mirror_impl_t<Tr0, EH>&, Traits,
                            typename Traits::rep&& value) const
      noexcept(std::is_nothrow_move_constructible_v<typename Traits::rep&&>) {
    return mirror_impl_t<Traits, EH>{(typename Traits::rep&&)value};
  }

  template <typename..., typename Ctx, typename T>
  constexpr auto operator()(Ctx&& ctx, T&& v) const CORDO_INTERNAL_RETURN_(
      (*this)((Ctx&&)ctx, mirror_traits_ctor(::cordo::tag_t<T&&>{}), (T&&)v));
};
inline constexpr make_mirror_impl_fn make_mirror_impl{};

struct make_mirror_result_fn final {
  template <typename..., typename T>
  constexpr decltype(auto) operator()(mirror_result<T>&& res) const
      CORDO_INTERNAL_RETURN_((mirror_result<T>&&)res);

  template <typename..., mirror_traits Traits, typename EH>
  constexpr decltype(auto) operator()(mirror_impl_t<Traits, EH>&& res) const
      CORDO_INTERNAL_RETURN_(EH::make_result((mirror_impl_t<Traits, EH>&&)res));
};
inline constexpr make_mirror_result_fn make_mirror_result{};

struct make_mirror_error_fn final {
  template <typename..., typename EH>
  constexpr auto operator()(EH, mirror_error err) const
      CORDO_INTERNAL_ALIAS_(EH::make_error(err));

  template <typename..., mirror_traits Tr0, typename EH>
  constexpr decltype(auto) operator()(const mirror_impl_t<Tr0, EH>&,
                                      mirror_error err) const
      CORDO_INTERNAL_RETURN_(EH::make_error(err));
};
inline constexpr make_mirror_error_fn make_mirror_error{};
}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_impl_t;

using cordo_internal_mirror::mirror_traits;
using cordo_internal_mirror::mirror_traits_ctor;
using cordo_internal_mirror::mirror_traits_name;
using cordo_internal_mirror::mirror_traits_of_const;
using cordo_internal_mirror::mirror_traits_subscript_keys;

using cordo_internal_mirror::mirror_assign;
using cordo_internal_mirror::mirror_has_value;
using cordo_internal_mirror::mirror_subscript_key;
using cordo_internal_mirror::mirror_unwrap;

template <typename Impl>
class mirror_t;

namespace cordo_internal_mirror {
struct make_mirror_api_fn final {
  template <mirror_traits Traits, typename EH>
  constexpr decltype(auto) operator()(mirror_impl_t<Traits, EH>&& impl) const
      noexcept(
          std::is_nothrow_constructible_v<mirror_t<mirror_impl_t<Traits, EH>>,
                                          mirror_impl_t<Traits, EH>&&>) {
    return mirror_t<mirror_impl_t<Traits, EH>>{std::move(impl)};
  }
};
inline constexpr make_mirror_api_fn make_mirror_api{};
}  // namespace cordo_internal_mirror

template <mirror_traits Traits, typename EH>
class mirror_t<mirror_impl_t<Traits, EH>> final {
  using T = typename Traits::t;
  using rep_t = typename Traits::rep;
  using impl_t = mirror_impl_t<Traits, EH>;
  impl_t impl_;

  constexpr const impl_t& impl() const noexcept { return impl_; }
  constexpr impl_t& impl() noexcept { return impl_; }

  // TODO: inspirational goal: subscript(non-key-index)
 public:
  using traits = Traits;

  constexpr mirror_t(mirror_t&&) = default;
  constexpr mirror_t(const mirror_t&) = default;
  constexpr mirror_t& operator=(mirror_t&&) = default;
  constexpr mirror_t& operator=(const mirror_t&) = default;

  explicit constexpr mirror_t(impl_t&& core) noexcept(
      std::is_nothrow_move_constructible_v<impl_t>)
      : impl_{(impl_t&&)core} {}

  constexpr const T& v() const noexcept
    requires(std::is_same_v<T&, rep_t>)
  {
    return impl_.value;
  }
  constexpr T& v() noexcept
    requires(std::is_same_v<T&, rep_t>)
  {
    return impl_.value;
  }

  template <typename..., typename S = const mirror_t&>
  constexpr decltype(auto) has_value() const
      CORDO_INTERNAL_RETURN_(cordo_internal_mirror::mirror_impl_apply(
          static_cast<S>(*this).impl(), mirror_has_value));

  template <typename..., typename S = mirror_t&>
  constexpr decltype(auto) unwrap() CORDO_INTERNAL_RETURN_(
      cordo_internal_mirror::mirror_impl_apply(static_cast<S>(*this).impl(),
                                               mirror_unwrap) |
      cordo::piped(cordo_internal_mirror::make_mirror_api));
  template <typename..., typename S = const mirror_t&>
  constexpr decltype(auto) unwrap() const CORDO_INTERNAL_RETURN_(
      cordo_internal_mirror::mirror_impl_apply(static_cast<S>(*this).impl(),
                                               mirror_unwrap) |
      cordo::piped(cordo_internal_mirror::make_mirror_api));

  template <typename..., typename U,
            typename = std::enable_if_t<
                !std::is_same_v<std::remove_cvref_t<U>, mirror_t>>>
  constexpr auto operator=(U&& v) noexcept(
      noexcept(cordo_internal_mirror::mirror_impl_apply(this->impl(),
                                                        mirror_assign, (U&&)v)))
      -> decltype(((void)cordo_internal_mirror::mirror_impl_apply(this->impl(),
                                                                  mirror_assign,
                                                                  (U&&)v)),
                  *this) {
    cordo_internal_mirror::mirror_impl_apply(this->impl(), mirror_assign,
                                             (U&&)v);
    return *this;
  }

  template <auto K>
  constexpr auto operator[](::cordo::key_t<K> k) const CORDO_INTERNAL_ALIAS_(
      cordo_internal_mirror::mirror_impl_apply(this->impl(),
                                               mirror_subscript_key, k) |
      cordo::piped(cordo_internal_mirror::make_mirror_api));

  template <auto K>
  constexpr auto operator[](::cordo::key_t<K> k) CORDO_INTERNAL_ALIAS_(
      cordo_internal_mirror::mirror_impl_apply(this->impl(),
                                               mirror_subscript_key, k) |
      cordo::piped(cordo_internal_mirror::make_mirror_api));
};

namespace cordo_internal_mirror {
struct is_mirror_impl final {
  template <typename I>
  static constexpr bool test(cordo::tag_t<cordo::mirror_t<I>>) noexcept {
    return true;
  }
  static constexpr bool test(...) noexcept { return false; }
};

struct mirror_fn final {
  template <typename T, typename EH = cordo::eh_terminate>
  constexpr auto operator()(T&& v, EH eh = {}) const CORDO_INTERNAL_RETURN_(
      make_mirror_impl(eh, (T&&)v) |
      cordo::piped(cordo_internal_mirror::make_mirror_api));
};
}  // namespace cordo_internal_mirror

template <typename T>
concept is_mirror = cordo_internal_mirror::is_mirror_impl::test(
    cordo::tag_t<std::remove_cvref_t<T>>{});

inline constexpr cordo_internal_mirror::mirror_fn mirror{};
}  // namespace cordo