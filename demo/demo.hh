#pragma once

#include "cordo/cordo.hh"
#include <string>

namespace demo {

struct __attribute__((annotate("cordo"))) Book final {
  std::string author;
  std::string title;
};

}  // namespace demo