constexpr auto cordo_cpo(
    ::cordo::mirror_cpo,
    ::cordo::tag_t<::demo::Book>) {
  constexpr struct {
    using tuple_t = ::demo::Book;
    using fields_t = ::cordo::values_t<
      ("author"_key = ::cordo::field(&::demo::Book::author)),
      ("title"_key = ::cordo::field(&::demo::Book::title))>;
    constexpr auto name() const noexcept { return "demo::Book"_key; }
    constexpr auto fields() const noexcept { return fields_t{}; }
    constexpr auto operator[](decltype("author"_key) k) const noexcept { return (k = ::cordo::field(&::demo::Book::author)); }
    constexpr auto operator[](decltype("title"_key) k) const noexcept { return (k = ::cordo::field(&::demo::Book::title)); }
  } result{};
  return result;
}
