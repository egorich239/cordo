#pragma once

#include <bit>
#include <climits>
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "cordo/impl/core/cpo.hh"
#include "cordo/impl/core/cstring.hh"
#include "cordo/impl/core/kv.hh"
#include "cordo/impl/core/meta.hh"
#include "cordo/impl/mirror.hh"

namespace cordo_internal_mirror {

template <typename T, typename I>
struct mirror_option final {
  using t = T;
  using inner_t = I;

  using name = ::cordo::null_t;
};

}  // namespace cordo_internal_mirror

namespace cordo_internal_cpo {

template <typename T, typename I>
CORDO_INTERNAL_LAMBDA_(  //
    cordo_cpo,           //
    (::cordo::mirror_traits_subscript_keys_cpo c, adl_tag t,
     ::cordo_internal_mirror::mirror_option<T, I>),  //
    (::cordo::invoke(                                //
        c, t,                                        //
        ::cordo::mirror.t(::cordo::tag_t<I>{}))));

// template <typename T, typename Traits, auto K>

//     cordo_cpo,           //
//     (::cordo::mirror_subscript_key_cpo, adl_tag, Traits, T& s,
//      ::cordo::key_t<K> k),  //


}