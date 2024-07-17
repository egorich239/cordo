#pragma once

#include <type_traits>

#include "cordo/impl/accessor.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/literal.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_struct {

template <typename D>
concept struct_meta = requires {
  requires std::is_default_constructible_v<D>;

  typename D::tuple_t;
  requires !std::is_reference_v<typename D::tuple_t>;

  typename D::fields_t;
  // TODO: fields properties.

  typename ::cordo::make_key<D{}.name()>;
};

template <::cordo::key_t Name, typename S, ::cordo::kv_t... Fields>
struct struct_ final {
  static_assert(
      (::cordo::accessor<decltype(::cordo::make_accessor(Fields.value()))> &&
       ...));

  using t = S;

  using tuple_t = S;
  using fields_t = ::cordo::values_t<Fields...>;

  constexpr auto name() const noexcept { return Name; }
};

}  // namespace cordo_internal_struct

namespace cordo {
using ::cordo_internal_struct::struct_;
}  // namespace cordo

namespace cordo_internal_cpo {

template <::cordo_internal_struct::struct_meta M, typename S, auto K>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::mirror_subscript_key_cpo, adl_tag, M, S& s,
     ::cordo::key_t<K> k),  //
    (::cordo::get(
        ::cordo::make_accessor(::cordo::kv_lookup(typename M::fields_t{}, k)),
        s)));

}