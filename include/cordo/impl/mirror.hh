#pragma once

#include <type_traits>

#include "cordo/impl/core/algo.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/macros.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/mirror_result.hh"

namespace cordo {
namespace cordo_internal_mirror {

template <typename Traits>
concept mirror_traits = requires {
  typename Traits::t;
  requires !std::is_reference_v<typename Traits::t>;
};

template <mirror_traits Traits, typename EH>
struct mirror_impl_t final {
  using rep_t = typename Traits::rep;

  static constexpr auto traits() noexcept { return Traits{}; }
  static constexpr auto eh() noexcept { return EH{}; }

  template <cordo::algo Algo, typename... Args>
      constexpr auto apply(Algo algo, Args&&... args) &
      CORDO_INTERNAL_ALIAS_(algo(Traits{}, *this, (Args&&)args...));

  template <cordo::algo Algo, typename... Args>
      constexpr auto apply(Algo algo, Args&&... args) &&
      CORDO_INTERNAL_ALIAS_(algo(Traits{}, std::move(*this), (Args&&)args...));

  template <cordo::algo Algo, typename... Args>
  constexpr auto apply(Algo algo, Args&&... args) const& CORDO_INTERNAL_ALIAS_(
      algo(Traits{}, *this, (Args&&)args...));

  rep_t value;
};

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
  template <mirror_traits Traits, typename EH, typename U>
  constexpr decltype(auto) operator()(mirror_impl_t<Traits, EH>& core,
                                      U&& value) const
      CORDO_INTERNAL_RETURN_(core.value = (U&&)value);
};

struct mirror_subscript_key_core_t final {};

struct mirror_unwrap_core_t final {};

inline constexpr ::cordo::algo_t<mirror_traits_ctor_core_t{}>
    mirror_traits_ctor;
inline constexpr ::cordo::algo_t<mirror_traits_name_t{}> mirror_traits_name;
inline constexpr ::cordo::algo_t<mirror_traits_subscript_keys_core_t{}>
    mirror_traits_subscript_keys;

inline constexpr ::cordo::algo_t<mirror_assign_t{}> mirror_assign;
inline constexpr ::cordo::algo_t<mirror_subscript_key_core_t{}>
    mirror_subscript_key;
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

template <mirror_traits Traits, typename EH>
class mirror_api final {
  using T = typename Traits::t;
  using rep = typename Traits::rep;
  using core_t = mirror_impl_t<Traits, EH>;
  core_t core_;

  constexpr const core_t& core() const noexcept { return core_; }
  constexpr core_t& core() noexcept { return core_; }

  template <auto K, typename C>
  static constexpr decltype(auto) subscript(::cordo::overload_prio_t<1>,
                                            C&& core, ::cordo::key_t<K> k)
      CORDO_INTERNAL_RETURN_(mirror_subscript_key((C&&)core, k));

  // TODO: inspirational goal: subscript(non-key-index)

  static constexpr struct make_api_fn final {
    template <typename T2, typename EH2>
    constexpr auto operator()(mirror_impl_t<T2, EH2>&& core) const noexcept {
      return mirror_api<T2, EH2>{(mirror_impl_t<T2, EH2>&&)core};
    }
  } make_api{};

 public:
  using traits = Traits;

  explicit constexpr mirror_api(core_t&& core) noexcept(
      std::is_nothrow_move_constructible_v<core_t>)
      : core_{(core_t&&)core} {}

  constexpr const T& v() const noexcept
    requires(std::is_same_v<T&, rep>)
  {
    return core_.value;
  }
  constexpr T& v() noexcept
    requires(std::is_same_v<T&, rep>)
  {
    return core_.value;
  }

  template <typename..., typename S = mirror_api&>
  constexpr auto unwrap()
      CORDO_INTERNAL_ALIAS_(mirror_unwrap(static_cast<S>(*this).core()) |
                            cordo::piped(mirror_api::make_api));
  template <typename..., typename S = const mirror_api&>
  constexpr auto unwrap() const
      CORDO_INTERNAL_ALIAS_(mirror_unwrap(static_cast<S>(*this).core()) |
                            cordo::piped(mirror_api::make_api));

  constexpr mirror_api(mirror_api&&) = default;
  constexpr mirror_api(const mirror_api&) = default;
  constexpr mirror_api& operator=(mirror_api&&) = default;
  constexpr mirror_api& operator=(const mirror_api&) = default;

  template <typename..., typename U,
            typename = std::enable_if_t<
                !std::is_same_v<std::remove_cvref_t<U>, mirror_api>>>
  constexpr auto operator=(U&& v)
      CORDO_INTERNAL_ALIAS_(((void)mirror_assign(this->core(), (U&&)v)), *this);

  template <typename K>
  constexpr auto operator[](K&& k) const
      CORDO_INTERNAL_ALIAS_(mirror_api::subscript(::cordo::overload_prio_t<1>{},
                                                  this->core(), (K&&)k) |
                            cordo::piped(mirror_api::make_api));

  template <typename K>
  constexpr auto operator[](K&& k)
      CORDO_INTERNAL_ALIAS_(mirror_api::subscript(::cordo::overload_prio_t<1>{},
                                                  this->core(), (K&&)k) |
                            cordo::piped(mirror_api::make_api));
};
template <typename Traits, typename EH>
mirror_api(mirror_impl_t<Traits, EH>&&) -> mirror_api<Traits, EH>;

struct mirror_fn final {
  template <typename T>
  constexpr auto t(::cordo::tag_t<T>) const
      CORDO_INTERNAL_RETURN_(mirror_traits_ctor(::cordo::tag_t<T>{}));

  template <typename T>
  constexpr auto traits(T&&) const
      CORDO_INTERNAL_RETURN_(this->t(::cordo::tag_t<T&&>{}));

  template <typename T, typename EH>
  constexpr auto core(T&& v, EH) const
      CORDO_INTERNAL_RETURN_(make_mirror_impl(EH{}, (T&&)v));

  template <typename T, typename EH = cordo::eh_terminate>
  constexpr auto operator()(T&& v, EH eh = {}) const CORDO_INTERNAL_RETURN_(
      mirror_api<decltype(this->traits((T&&)v)), EH>(this->core((T&&)v, eh)));
};
}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_api;
using cordo_internal_mirror::mirror_impl_t;

using cordo_internal_mirror::mirror_traits_ctor;
using cordo_internal_mirror::mirror_traits_name;
using cordo_internal_mirror::mirror_traits_of_const;
using cordo_internal_mirror::mirror_traits_subscript_keys;

using cordo_internal_mirror::mirror_assign;
using cordo_internal_mirror::mirror_subscript_key;
using cordo_internal_mirror::mirror_unwrap;

inline constexpr cordo_internal_mirror::mirror_fn mirror{};
}  // namespace cordo