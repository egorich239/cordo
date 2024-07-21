#pragma once

#define CORDO_INTERNAL_INLINE2_(...) __VA_ARGS__
#define CORDO_INTERNAL_INLINE_(p) CORDO_INTERNAL_INLINE2_ p

#define CORDO_INTERNAL_ALIAS_(...)                         \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) { \
    return __VA_ARGS__;                                    \
  }                                                        \
  static_assert(true)

#define CORDO_INTERNAL_LAMBDA_R_(name, args, body, reqs)                    \
  constexpr auto name args noexcept(noexcept(CORDO_INTERNAL_INLINE_(body))) \
      ->decltype(CORDO_INTERNAL_INLINE_(body)) reqs {                       \
    return CORDO_INTERNAL_INLINE_(body);                                    \
  }                                                                         \
  static_assert(true)

#define CORDO_INTERNAL_LAMBDA_(name, args, body) \
  CORDO_INTERNAL_LAMBDA_R_(name, args, body, )

#define CORDO_INTERNAL_DIAG_(diag) \
  ([]<bool flag>() { static_assert(flag, diag); }).template operator()<false>()