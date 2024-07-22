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
  constexpr auto operator()(::cordo::tag_t<T>) const noexcept {
    return mirror_unsupported<T>{};
  }
};

struct mirror_traits_name_cpo_t final {
 private:
  template <typename T>
  CORDO_INTERNAL_LAMBDA_(                      //
      resolve,                                 //
      (::cordo::overload_prio_t<1>, T) const,  //
      (typename T::name{}));

  template <typename T>
  CORDO_INTERNAL_LAMBDA_(                      //
      resolve,                                 //
      (::cordo::overload_prio_t<0>, T) const,  //
      (::cordo::null_t{}));

 public:
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  template <typename T>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(),          //
      (T v) const,         //
      (this->resolve(::cordo::overload_prio_t<1>{}, v)));
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
  constexpr auto operator()(T v) const
      CORDO_INTERNAL_ALIAS_(this->resolve(::cordo::overload_prio_t<1>{}, v));
};

struct mirror_make_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  template <mirror_traits Traits, typename... Args>
  CORDO_INTERNAL_LAMBDA_(              //
      operator(),                      //
      (Traits, Args&&... args) const,  //
      (typename Traits::t{(Args&&)args...}));
};

struct mirror_assign_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  template <mirror_traits Traits, typename T, typename U>
  CORDO_INTERNAL_LAMBDA_(                    //
      operator(),                            //
      (Traits, T& target, U&& value) const,  //
      (target = (U&&)value));

  template <mirror_traits Traits, typename T, typename U>
  void operator()(Traits, T&& target, U&& value) const = delete;
};

struct mirror_subscript_key_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;
};

struct mirror_subscript_index_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
};

struct mirror_unwrap_t final {
  using adl_tag = ::cordo_internal_cpo::adl_tag;
};
inline constexpr ::cordo::algo<mirror_unwrap_t> mirror_unwrap;

inline constexpr ::cordo::algo<mirror_traits_ctor_t> mirror_traits_ctor;

using mirror_traits_name_cpo = ::cordo::cpo_t<mirror_traits_name_cpo_t{}>;

inline constexpr ::cordo::algo<mirror_traits_subscript_keys_t>
    mirror_traits_subscript_keys{};

using mirror_make_cpo = ::cordo::cpo_t<mirror_make_cpo_t{}>;

using mirror_assign_cpo = ::cordo::cpo_t<mirror_assign_cpo_t{}>;
inline constexpr ::cordo::algo<mirror_subscript_key_t> mirror_subscript_key{};
using mirror_subscript_index_cpo =
    ::cordo::cpo_t<mirror_subscript_index_cpo_t{}>;

template <mirror_traits Traits, typename M>
struct mirror_ref final {
  M* mirror;

  constexpr M& operator*() const noexcept { return *mirror; }
  constexpr M* operator->() const noexcept { return mirror; }

  constexpr decltype(auto) rep() const noexcept { return mirror->value_; }
};

template <mirror_traits Traits, typename Fn>
class mirror_t final {
  using T = typename Traits::t;
  using rep = typename Traits::rep;
  rep value_;

  template <auto K, typename U>
  static constexpr auto subscript(::cordo::overload_prio_t<1>, Traits traits,
                                  U& value, ::cordo::key_t<K> k)
      CORDO_INTERNAL_ALIAS_(Fn{}(mirror_subscript_key(Traits{}, value, k)));

  template <typename K, typename U>
  static CORDO_INTERNAL_LAMBDA_(                                      //
      subscript,                                                      //
      (::cordo::overload_prio_t<0>, Traits traits, U& value, K&& k),  //
      (Fn{}(::cordo::invoke(mirror_subscript_index_cpo{}, Traits{}, value,
                            (K&&)k))));

 public:
  using traits = Traits;

  explicit constexpr mirror_t(rep&& value, Traits, Fn) noexcept
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

  constexpr decltype(auto) unwrap() noexcept(
      noexcept(Fn{}(mirror_unwrap(Traits{}, (rep&&)this->value_))))
    requires(sizeof(decltype(mirror_unwrap(Traits{},
                                           (rep &&) this -> value_))) >= 1)
  {
    return Fn{}(mirror_unwrap(Traits{}, (rep&&)this->value_));
  }

  template <typename U>
  CORDO_INTERNAL_LAMBDA_(  //
      operator=,           //
      (U&& v),             //
      (((void)::cordo::invoke(mirror_assign_cpo{}, Traits{},
                              (rep&&)this->value_, (U&&)v)),
       *this));

  template <typename K>
  constexpr auto operator[](K&& k) const
      CORDO_INTERNAL_ALIAS_(mirror_t::subscript(::cordo::overload_prio_t<1>{},
                                                Traits{}, this->value_,
                                                (K&&)k));

  template <typename K>
  constexpr auto operator[](K&& k)
      CORDO_INTERNAL_ALIAS_(mirror_t::subscript(::cordo::overload_prio_t<1>{},
                                                Traits{}, this->value_,
                                                (K&&)k));

 private:
  friend class mirror_ref<traits, mirror_t>;
  friend class mirror_ref<traits, const mirror_t>;
};
template <typename Traits, typename Fn>
mirror_t(typename Traits::rep&&, Traits, Fn) -> mirror_t<Traits, Fn>;

struct mirror_fn final {
  template <typename T>
  constexpr auto t(::cordo::tag_t<T>) const
      CORDO_INTERNAL_RETURN_(mirror_traits_ctor(::cordo::tag_t<T>{}));

  template <typename T>
  constexpr auto traits(T&&) const
      CORDO_INTERNAL_RETURN_(this->t(::cordo::tag_t<T>{}));

  template <typename T>
  constexpr auto operator()(T&& v) const
      CORDO_INTERNAL_RETURN_(mirror_t((T&&)v, this->traits((T&&)v), *this));

  template <typename T, typename... Args>
  CORDO_INTERNAL_LAMBDA_(      //
      make,                    //
      (Args&&... args) const,  //
      (::cordo::invoke(mirror_make_cpo{}, this->t(::cordo::tag_t<T>{}),
                       (Args&&)args...)));
};
}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirror_traits_ctor;
using ::cordo_internal_mirror::mirror_traits_name_cpo;
using ::cordo_internal_mirror::mirror_traits_subscript_keys;

using ::cordo_internal_mirror::mirror_make_cpo;

using ::cordo_internal_mirror::mirror_assign_cpo;
using ::cordo_internal_mirror::mirror_subscript_index_cpo;
using ::cordo_internal_mirror::mirror_subscript_key;
using ::cordo_internal_mirror::mirror_unwrap;

inline constexpr ::cordo_internal_mirror::mirror_fn mirror{};
}  // namespace cordo