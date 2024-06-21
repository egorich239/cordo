#pragma once

#define CORDO_INTERNAL_INLINE2_(...) __VA_ARGS__
#define CORDO_INTERNAL_INLINE_(p) CORDO_INTERNAL_INLINE2_ p
#define CORDO_INTERNAL_LAMBDA_(name, args, body)              \
  constexpr auto name(CORDO_INTERNAL_INLINE_(args))           \
      const noexcept(noexcept(CORDO_INTERNAL_INLINE_(         \
          body))) -> decltype(CORDO_INTERNAL_INLINE_(body)) { \
    return CORDO_INTERNAL_INLINE_(body);                      \
  }                                                           \
  static_assert(true)