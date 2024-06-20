#pragma once

#include <cstddef>

namespace cordo_internal_algo {
template <auto A>
struct algo_t final {};
}  // namespace cordo_internal_algo

namespace cordo {
using ::cordo_internal_algo::algo_t;
struct adl_hook_t final {};
}  // namespace cordo

namespace cordo_internal_algo {
template <std::size_t N>
struct overload_prio_t : overload_prio_t<N - 1> {};
template <>
struct overload_prio_t<0> {};

struct invoke_t final {
  template <auto A, typename... Args>
  constexpr decltype(auto) operator()(const algo_t<A> &a,
                                      Args &&...args) const {
    return this->resolve(overload_prio_t<2>{}, a, (Args &&)args...);
  }

 private:
  template <auto A, typename... Args>
  constexpr auto resolve(overload_prio_t<2>, const algo_t<A> &a, Args &&...args)
      const -> decltype(cordo_algo(a, (Args &&)args...)) {
    return cordo_algo(a, (Args &&)args...);
  }
  template <auto A, typename... Args>
  constexpr auto resolve(overload_prio_t<1>, const algo_t<A> &a,
                         Args &&...args) const
      -> decltype(cordo_algo(a, ::cordo::adl_hook_t{}, (Args &&)args...)) {
    return cordo_algo(a, ::cordo::adl_hook_t{}, (Args &&)args...);
  }
};
}  // namespace cordo_internal_algo

namespace cordo {
inline constexpr ::cordo_internal_algo::invoke_t invoke{};
}  // namespace cordo
