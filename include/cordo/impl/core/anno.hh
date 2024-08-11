#pragma once

#include "cordo/impl/core/invoke.hh"
#include "cordo/impl/core/meta.hh"

namespace cordo_internal_anno {

template <typename Fn>
struct anno_fn final {
  Fn fn;
};

template <typename L, typename Fn>
struct anno_apply final {
  L input;
  anno_fn<Fn> anno;
};

template <typename L, typename Fn>
constexpr auto operator||(L input, anno_fn<Fn> fn) {
  return anno_apply{input, fn};
}

template <typename P>
struct anno_t {
  template <typename... Args>
  constexpr auto operator()(Args... args) const noexcept {
    const P self = static_cast<const P&>(*this);
    return anno_fn{[self, args...](auto fn) { return fn(self, args...); }};
  }
};

struct anno_eval_fn final {
  template <typename L, typename P, typename Sema>
  CORDO_INTERNAL_LAMBDA_(                        //
      operator(),                                //
      (Sema sema, anno_apply<L, P> expr) const,  //
      (sema.apply((*this)(sema, expr), expr.anno.fn)));

  template <typename V, typename Sema>
  CORDO_INTERNAL_LAMBDA_(         //
      operator(),                 //
      (Sema sema, V expr) const,  //
      (sema.init(expr)));
};

}  // namespace cordo_internal_anno

namespace cordo {
inline constexpr ::cordo_internal_anno::anno_eval_fn anno{};
}  // namespace cordo