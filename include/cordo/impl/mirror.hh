#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/macros.hh"

namespace cordo_internal_mirror {

struct mirror_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
};

struct mirror_construct_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }
};

struct mirror_subscript_cpo_t final {
  constexpr ::cordo_internal_cpo::adl_tag adl_tag() const noexcept {
    return {};
  }

  template <typename S, typename T, typename K>
  CORDO_INTERNAL_LAMBDA_(       //
      operator(),               //
      (S&& s, T t, K k) const,  //
      (::cordo::get(
          ::cordo::make_accessor(::cordo::kv_lookup(typename T::fields_t{}, k)),
          s)));
};

using mirror_cpo = ::cordo::cpo_t<mirror_cpo_t{}>;
using mirror_construct_cpo = ::cordo::cpo_t<mirror_construct_cpo_t{}>;
using mirror_subscript_cpo = ::cordo::cpo_t<mirror_subscript_cpo_t{}>;

struct mirror_t final {
  template <typename T>
  CORDO_INTERNAL_LAMBDA_(         //
      t,                          //
      (::cordo::tag_t<T>) const,  //
      (::cordo::invoke(mirror_cpo{},
                       ::cordo::tag_t<std::remove_cvref_t<T>>{})));

  template <typename S>
  CORDO_INTERNAL_LAMBDA_(  //
      operator(),          //
      (S& v) const,        //
      (::cordo::invoke(mirror_construct_cpo{},
                       this->t(::cordo::tag_t<std::remove_cvref_t<S>>{}), v)));
};
}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirror_construct_cpo;
using ::cordo_internal_mirror::mirror_cpo;
using ::cordo_internal_mirror::mirror_subscript_cpo;
inline constexpr ::cordo_internal_mirror::mirror_t mirror{};
}  // namespace cordo

namespace cordo_internal_mirror {

template <typename T>
concept mirrored = std::is_same_v<                                       //
    typename decltype(::cordo::mirror.t(::cordo::tag_t<T>{}))::tuple_t,  //
    std::remove_cvref_t<T>>;

}  // namespace cordo_internal_mirror

namespace cordo {
using ::cordo_internal_mirror::mirrored;
}  // namespace cordo