#pragma once
#include "cordo/cordo.hh"

namespace demo {
constexpr auto cordo_cpo(
    ::cordo::mirror_cpo,
    ::cordo::tag_t<::demo::Book>) noexcept {
  constexpr struct {
    using tuple_t = ::demo::Book;
    using fields_t = ::cordo::values_t<
      ("author"_key <= &::demo::Book::author),
      ("title"_key <= &::demo::Book::title)>;
    constexpr auto name() const noexcept { return "demo::Book"_key; }
  } result{};
  return result;
}
}  // namespace demo
