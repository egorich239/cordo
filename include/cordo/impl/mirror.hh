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
struct mirror_core final {
  using rep = typename Traits::rep;
  using eh = EH;

  static constexpr auto traits() noexcept { return Traits{}; }

  rep value;
};

template <typename T>
struct mirror_unsupported final {
  using t = T;
};

struct mirror_traits_of_const_t {};
inline constexpr ::cordo::algo<mirror_traits_of_const_t> mirror_traits_of_const;

struct mirror_traits_ctor_t final {
  // The idea of unsupported type is to simplify codegens for structs:
  // every field will get a trait, worst case it will be unsupported.
  // This however still allows us to later filter them with cordo::skip().
  template <typename T>
  constexpr auto operator()(const ::cordo::algo<mirror_traits_ctor_t>&,
                            ::cordo::tag_t<T>) const noexcept {
    return mirror_unsupported<T>{};
  }

  template <typename T>
  constexpr auto operator()(const ::cordo::algo<mirror_traits_ctor_t>& algo,
                            ::cordo::tag_t<T&>) const
      CORDO_INTERNAL_ALIAS_(algo(::cordo::tag_t<T>{}));

  template <typename T>
  constexpr auto operator()(const ::cordo::algo<mirror_traits_ctor_t>& algo,
                            ::cordo::tag_t<const T>) const
      CORDO_INTERNAL_ALIAS_(mirror_traits_of_const(algo(::cordo::tag_t<T>{})));

  // TODO: mirror_inlined?
  template <typename T>
  constexpr auto operator()(const ::cordo::algo<mirror_traits_ctor_t>& algo,
                            ::cordo::tag_t<T&&>) const noexcept = delete;
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
  template <typename T>
  constexpr auto operator()(
      const ::cordo::algo<mirror_traits_subscript_keys_t>&, T v) const
      CORDO_INTERNAL_ALIAS_(this->resolve(::cordo::overload_prio_t<1>{}, v));
};

struct mirror_assign_t final {
  template <mirror_traits Traits, typename EH, typename U>
  constexpr decltype(auto) operator()(const ::cordo::algo<mirror_assign_t>&,
                                      mirror_core<Traits, EH>& core,
                                      U&& value) const
      CORDO_INTERNAL_RETURN_(core.value = (U&&)value);
};

struct mirror_subscript_key_t final {};

struct mirror_unwrap_t final {};

inline constexpr ::cordo::algo<mirror_traits_ctor_t> mirror_traits_ctor;
inline constexpr ::cordo::algo<mirror_traits_name_t> mirror_traits_name;
inline constexpr ::cordo::algo<mirror_traits_subscript_keys_t>
    mirror_traits_subscript_keys;

inline constexpr ::cordo::algo<mirror_assign_t> mirror_assign;
inline constexpr ::cordo::algo<mirror_subscript_key_t> mirror_subscript_key;
inline constexpr ::cordo::algo<mirror_unwrap_t> mirror_unwrap;

template <mirror_traits Traits, typename EH>
class mirror_api final {
  using T = typename Traits::t;
  using rep = typename Traits::rep;
  using core_t = mirror_core<Traits, EH>;
  core_t core_;

 public:  // TODO
  constexpr const core_t& core() const noexcept { return core_; }
  constexpr core_t& core() noexcept { return core_; }

  template <auto K, typename C>
  static constexpr decltype(auto) subscript(::cordo::overload_prio_t<1>,
                                            C&& core, ::cordo::key_t<K> k)
      CORDO_INTERNAL_RETURN_(mirror_subscript_key((C&&)core, k));

  // TODO: inspirational goal: subscript(non-key-index)

  template <typename T2, typename EH2>
  static constexpr mirror_api<T2, EH2> make_api(
      mirror_core<T2, EH2>&& core) noexcept {
    return mirror_api<T2, EH2>{(mirror_core<T2, EH2>&&)core};
  }

  // TODO: generalize for other result types?
  template <typename T2>
  static constexpr auto make_api(cordo::mirror_result<T2>&& res) noexcept {
    return res.ok() ? cordo::mirror_result(make_api(std::move(res).value()))
                    : res.error();
  }

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

  template <typename..., typename R = mirror_core<Traits, EH>&,
            typename = decltype(mirror_unwrap(std::declval<R>()))>
  constexpr decltype(auto) unwrap()
      CORDO_INTERNAL_RETURN_(mirror_api::make_api(mirror_unwrap(this->core())));
  template <typename..., typename R = const mirror_core<Traits, EH>&,
            typename = decltype(mirror_unwrap(std::declval<R>()))>
  constexpr decltype(auto) unwrap() const
      CORDO_INTERNAL_RETURN_(mirror_api::make_api(mirror_unwrap(this->core())));

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
      CORDO_INTERNAL_ALIAS_(mirror_api::make_api(mirror_api::subscript(
          ::cordo::overload_prio_t<1>{}, this->core(), (K&&)k)));

  template <typename K>
  constexpr auto operator[](K&& k)
      CORDO_INTERNAL_ALIAS_(mirror_api::make_api(mirror_api::subscript(
          ::cordo::overload_prio_t<1>{}, this->core(), (K&&)k)));
};
template <typename Traits, typename EH>
mirror_api(mirror_core<Traits, EH>&&) -> mirror_api<Traits, EH>;

struct mirror_fn final {
  template <typename T>
  constexpr auto t(::cordo::tag_t<T>) const
      CORDO_INTERNAL_RETURN_(mirror_traits_ctor(::cordo::tag_t<T>{}));

  template <typename T>
  constexpr auto traits(T&&) const
      CORDO_INTERNAL_RETURN_(this->t(::cordo::tag_t<T&&>{}));

  template <typename T, typename EH>
  constexpr auto core(T&& v, EH) const
      CORDO_INTERNAL_RETURN_(mirror_core<decltype(this->traits((T&&)v)), EH>{
          static_cast<typename decltype(this->traits((T&&)v))::rep>((T&&)v)});

  template <typename T, typename EH = cordo::eh_terminate>
  constexpr auto operator()(T&& v, EH eh = {}) const CORDO_INTERNAL_RETURN_(
      mirror_api<decltype(this->traits((T&&)v)), EH>(this->core((T&&)v, eh)));
};
}  // namespace cordo_internal_mirror

using cordo_internal_mirror::mirror_api;
using cordo_internal_mirror::mirror_core;

using cordo_internal_mirror::mirror_traits_ctor;
using cordo_internal_mirror::mirror_traits_name;
using cordo_internal_mirror::mirror_traits_of_const;
using cordo_internal_mirror::mirror_traits_subscript_keys;

using cordo_internal_mirror::mirror_assign;
using cordo_internal_mirror::mirror_subscript_key;
using cordo_internal_mirror::mirror_unwrap;

inline constexpr cordo_internal_mirror::mirror_fn mirror{};
}  // namespace cordo