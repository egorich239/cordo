#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

#include "cordo/impl/cpo.hh"
#include "cordo/impl/kv.hh"
#include "cordo/impl/meta.hh"

namespace cordo_internal_any {

// template <typename... E>
struct any_storage_t final {
 public:
  ~any_storage_t() { dtor_(&storage_[0]); }

  template <typename T>
  static any_storage_t make(T&& v)
    requires(24 >= sizeof(T))
  {
    using V = std::remove_cvref_t<T>;
    return any_storage_t(::cordo::typeid_t<V>{}, (T&&)v);
  }

  template <typename T>
  const T* as() const noexcept {
    if (&::cordo::typeid_t<T>::key != typeid_) return nullptr;
    return std::launder(reinterpret_cast<const T*>(&storage_[0]));
  }
  template <typename T>
  T* as() noexcept {
    if (&::cordo::typeid_t<T>::key != typeid_) return nullptr;
    return std::launder(reinterpret_cast<T*>(&storage_[0]));
  }

 private:
  template <typename T, typename U>
  explicit any_storage_t(::cordo::typeid_t<T> id, U&& v)
    requires(!std::is_reference_v<T> &&
             std::is_same_v<T, std::remove_cvref_t<U>> &&
             std::is_constructible_v<T, U &&> && sizeof(T) <= 24)
      : storage_{}, typeid_{&decltype(id)::key}, dtor_{} {
    new (storage_) T{(U&&)v};
    dtor_ = +[](void* self) { std::launder(((T*)self))->~T(); };
  }

  alignas(std::max_align_t) std::byte storage_[24];
  void (*dtor_)(void*);
  const char* typeid_;
};

struct cpo_erasure_traits final {
  template <typename C, typename T, typename... Args>
  static constexpr ::cordo::tag_t<any_storage_t (*)(T, Args...)> prefix_(
      ::cordo::tag_t<T>, ::cordo::tag_t<C(Args...)>) noexcept {
    return {};
  }

  template <typename Erasure>
  using const_cb_t = typename decltype(prefix_(
      ::cordo::tag_t<const void*>{}, ::cordo::tag_t<Erasure>()))::type;
  template <typename Erasure>
  using mut_cb_t = typename decltype(prefix_(::cordo::tag_t<void*>{},
                                             ::cordo::tag_t<Erasure>()))::type;
};

template <typename Erasure>
struct any_cpo_cb_t final {
  cpo_erasure_traits::const_cb_t<Erasure> const_;
  cpo_erasure_traits::mut_cb_t<Erasure> mut_;
};

inline constexpr struct {
 private:
  template <typename C, typename T, typename E, typename... Args,
            typename = decltype(any_storage_t::make(::cordo::invoke(
                C{}, std::declval<T>(), std::declval<Args>()...)))>
  constexpr auto resolve(::cordo::overload_prio_t<1>, ::cordo::tag_t<T>,
                         ::cordo::tag_t<E>,
                         ::cordo::tag_t<C(Args...)>) const noexcept {
    return +[](E* v, Args... a) {
      return any_storage_t::make(
          ::cordo::invoke(C{}, *std::launder((T*)v), (Args&&)a...));
    };
  }
  template <typename C, typename T, typename E, typename... Args,
            typename = decltype(any_storage_t::make(::cordo::invoke(
                C{}, std::declval<T>(), std::declval<Args>()...)))>
  constexpr auto resolve(::cordo::overload_prio_t<0>, ::cordo::tag_t<T>,
                         ::cordo::tag_t<E>,
                         ::cordo::tag_t<C(Args...)>) const noexcept {
    return nullptr;
  }

 public:
  template <typename T, typename Erasure>
  constexpr any_cpo_cb_t<Erasure> operator()(
      ::cordo::tag_t<T>, ::cordo::tag_t<Erasure> e) const noexcept {
    return {
        .const_ = this->resolve(::cordo::overload_prio_t<1>{},
                                ::cordo::tag_t<const T&>{},
                                ::cordo::tag_t<const void*>{}, e),
        .mut_ = this->resolve(::cordo::overload_prio_t<1>{},
                              ::cordo::tag_t<T&>{}, ::cordo::tag_t<void*>{}, e),
    };
  }
} make_any_cpo_cb{};

}  // namespace cordo_internal_any

namespace cordo {
using any = ::cordo_internal_any::any_storage_t;
}  // namespace cordo
