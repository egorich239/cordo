#pragma once

#include <string>

#include "cordo/cordo.hh"

namespace demo {
using cordo::literals::operator""_key;

struct __attribute__((annotate("cordo", "foo"_key))) Book final {
  std::string author;
  std::string title;
};

}  // namespace demo