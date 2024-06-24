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
    dtor_ = +[](void* self) { std::launder((T*)self)->~T(); };
  }

  alignas(std::max_align_t) std::byte storage_[24];
  void (*dtor_)(void*) = nullptr;
  const char* typeid_ = nullptr;
};

template <typename T, typename Erasure>
struct cpo_cb_t_impl final {
  static_assert(std::is_same_v<T, void> || std::is_same_v<T, const void>);

 private:
  template <typename R, typename C, typename... Args>
  static constexpr ::cordo::tag_t<R (*)(C, T*, Args...)> impl(
      ::cordo::tag_t<R(C, Args...)>) noexcept {
    return {};
  }

 public:
  using type = typename decltype(impl(::cordo::tag_t<Erasure>{}))::type;
};
template <typename T, typename Erasure>
using cpo_cb_t = typename cpo_cb_t_impl<T, Erasure>::type;

inline constexpr struct {
 private:
  template <typename R, typename C, typename T, typename... Args>
  constexpr auto impl(::cordo::tag_t<T>,
                      ::cordo::tag_t<R(C, Args...)>) const noexcept {
    using BaseT = std::remove_reference_t<T>;
    return +[](C c, ::cordo::same_constness_as_t<BaseT, void>* v, Args... a) {
      return R{::cordo::invoke(c, *std::launder((BaseT*)v), (Args&&)a...)};
    };
  }

 public:
  template <typename T, typename Erasure>
  constexpr auto operator()(::cordo::tag_t<T> t,
                            ::cordo::tag_t<Erasure> e) const noexcept {
    return this->impl(t, e);
  }
} make_cpo_cb{};

template <typename... Erasures>
class any final {
  static constexpr size_t SIZE = sizeof...(Erasures);
  any_storage_t storage_;
  std::tuple<cpo_cb_t<const void, Erasures>...> cpos_const_;
  std::tuple<cpo_cb_t<void, Erasures>...> cpos_mut_;

 private:
  // NOTE: inlining fn_{const_,mut_} breaks SFINAE for clang-14.
  template <::std::size_t N>
  auto fn_const_() const noexcept -> decltype(auto)
    requires(N < SIZE)
  {
    return std::get<N>(this->cpos_const_);
  }
  template <::std::size_t N>
  auto fn_mut_() const noexcept -> decltype(auto)
    requires(N < SIZE)
  {
    return std::get<N>(this->cpos_mut_);
  }

  template <::std::size_t N, typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(  //
      try_invoke,            //
      (::cordo::overload_prio_t<1>, ::cordo::value_t<N>, C c, Args&&... args)
          const,  //
      (this->fn_const_<SIZE - N>()(c, this->storage_.storage(),
                                   (Args&&)args...)),
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
      (this->fn_mut_<SIZE - N>()(c, this->storage_.storage(), (Args&&)args...)),
      requires(N != 0));

  template <::std::size_t N, typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(  //
      try_invoke,            //
      (::cordo::overload_prio_t<0>, ::cordo::value_t<N>, C c,
       Args&&... args),  //
      (this->try_invoke(::cordo::overload_prio_t<1>{},
                        ::cordo::value_t<N - 1>{}, c, (Args&&)args...)),
      requires(N > 1));

 public:
  template <typename T>
  explicit any(T&& v)
      : storage_{any_storage_t::make((T&&)v)},
        cpos_const_{make_cpo_cb(::cordo::tag_t<const std::remove_cvref_t<T>>{},
                                ::cordo::tag_t<Erasures>{})...},
        cpos_mut_{make_cpo_cb(::cordo::tag_t<std::remove_cvref_t<T>>{},
                              ::cordo::tag_t<Erasures>{})...} {}

  template <typename T>
  const T* as() const noexcept {
    return storage_.as<T>();
  }
  template <typename T>
  T* as() noexcept {
    return storage_.as<T>();
  }

  template <typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(         //
      invoke,                       //
      (C c, Args&&... args) const,  //
      (this->try_invoke(::cordo::overload_prio_t<2>{}, ::cordo::value_t<SIZE>{},
                        c, (Args&&)args...)),  //
      requires(SIZE > 0));

  template <typename C, typename... Args>
  CORDO_INTERNAL_LAMBDA_R_(   //
      invoke,                 //
      (C c, Args&&... args),  //
      (this->try_invoke(::cordo::overload_prio_t<2>{}, ::cordo::value_t<SIZE>{},
                        c, (Args&&)args...)),  //
      requires(SIZE > 0));
};
}  // namespace cordo_internal_any

namespace cordo {
using ::cordo_internal_any::any;
using any_v = any<>;
}  // namespace cordo
