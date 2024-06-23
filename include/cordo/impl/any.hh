#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <tuple>
#include <type_traits>

#include "cordo/impl/cpo.hh"
#include "cordo/impl/kv.hh"
#include "cordo/impl/meta.hh"

namespace cordo_internal_any {

// template <typename... E>
struct any_storage_t final {
 public:
  constexpr any_storage_t() noexcept = default;
  ~any_storage_t() {
    if (dtor_) dtor_(storage());
  }

  template <typename T>
  static any_storage_t make(T&& v)
    requires(24 >= sizeof(T))
  {
    using V = std::remove_cvref_t<T>;
    return any_storage_t(::cordo::typeid_t<V>{}, (T&&)v);
  }

  const void* storage() const noexcept { return &storage_[0]; }
  void* storage() noexcept { return &storage_[0]; }

  template <typename T>
  const T* as() const noexcept {
    if (&::cordo::typeid_t<T>::key != typeid_) return nullptr;
    return std::launder(reinterpret_cast<const T*>(storage()));
  }
  template <typename T>
  T* as() noexcept {
    if (&::cordo::typeid_t<T>::key != typeid_) return nullptr;
    return std::launder(reinterpret_cast<T*>(storage()));
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
  void (*dtor_)(void*) = nullptr;
  const char* typeid_ = nullptr;
};

struct cpo_erasure_traits final {
  template <typename C, typename T, typename... Args>
  static constexpr ::cordo::tag_t<any_storage_t (*)(C, T, Args...)> prefix_(
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
    return +[](C c, E v, Args... a) {
      return any_storage_t::make(::cordo::invoke(
          c, *std::launder((std::remove_reference_t<T>*)v), (Args&&)a...));
    };
  }
    template <typename C, typename T, typename E, typename... Args>
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

template <typename... Erasures>
struct any final {
  template <typename T>
  explicit any(T&& v)
      : storage_{any_storage_t::make((T&&)v)},
        cpos_{make_any_cpo_cb(::cordo::tag_t<std::remove_cvref_t<T>>{},
                              ::cordo::tag_t<Erasures>{})...} {}

  explicit any(any_storage_t&& s)
    requires(sizeof...(Erasures) == 0)
      : storage_{(any_storage_t)s} {}

  template <typename T>
  const T* as() const noexcept {
    return storage_.as<T>();
  }
  template <typename T>
  T* as() noexcept {
    return storage_.as<T>();
  }

  template <typename C, typename... Args>
  any<> invoke(C c, Args&&... args) const {
    return this->try_invoke(::cordo::overload_prio_t<1>{},
                            ::cordo::value_t<sizeof...(Erasures)>{}, c,
                            (Args&&)args...);
  }

 private:
  any_storage_t storage_;
  std::tuple<any_cpo_cb_t<Erasures>...> cpos_;

 private:
  template <::std::size_t N, typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(  //
      try_invoke,            //
      (::cordo::overload_prio_t<1>, ::cordo::value_t<N>, C c, Args&&... args)
          const,  //
      (any<>(std::get<sizeof...(Erasures) - N>(this->cpos_)
                 .const_(c, this->storage_.storage(), (Args&&)args...))),
      requires(N != 0));

  template <::std::size_t N, typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(  //
      try_invoke,            //
      (::cordo::overload_prio_t<0>, ::cordo::value_t<N>, C c, Args&&... args)
          const,  //
      (this->try_invoke(::cordo::overload_prio_t<1>{},
                        ::cordo::value_t<N - 1>{}, c, (Args&&)args...)),
      requires(N > 1));

  template <::std::size_t N, typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(  //
      try_invoke,            //
      (::cordo::overload_prio_t<1>, ::cordo::value_t<N>, C c,
       Args&&... args),  //
      (any<>(std::get<sizeof...(Erasures) - N>(this->cpos_)
                 .mut_(c, this->storage_.storage(), (Args&&)args...))),
      requires(N != 0));

  template <::std::size_t N, typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(  //
      try_invoke,            //
      (::cordo::overload_prio_t<0>, ::cordo::value_t<N>, C c,
       Args&&... args),  //
      (this->try_invoke(::cordo::overload_prio_t<1>{},
                        ::cordo::value_t<N - 1>{}, c, (Args&&)args...)),
      requires(N > 1));
};
}  // namespace cordo_internal_any

namespace cordo {
using ::cordo_internal_any::any;
}  // namespace cordo
