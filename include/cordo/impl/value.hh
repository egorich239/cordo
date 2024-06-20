#pragma once

#include <type_traits>

namespace cordo {
namespace cordo_value_internal {
template <auto V>
struct value_t final {
  using type = decltype(V);
  constexpr decltype(auto) operator()() const noexcept { return V; }
};
}  // namespace cordo_value_internal

using ::cordo::cordo_value_internal::value_t;

}  // namespace cordo