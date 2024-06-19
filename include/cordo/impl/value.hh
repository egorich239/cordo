#pragma once

#include <type_traits>

namespace cordo {

template <auto V>
using value_t = std::integral_constant<decltype(V), V>;

}