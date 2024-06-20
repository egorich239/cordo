#include <iostream>

#include "cordo/cordo.hh"

struct Foo {
  int x;

  int random() const { return 6; }
  const int& x_const() const { return x; }
  int& x_mut() { return x; }
};

struct Baz {
  Foo l;
};

int main(int argc, const char** argv) {
  using namespace ::cordo::literals;
  std::cout << "foo"_t() << "\n";

  constexpr auto acc = cordo::field_(&Foo::x);
  constexpr auto x_field = cordo::named_("x"_t, cordo::field_(&Foo::x));

  Foo a{.x = 3};
  std::cout << a.x << "\n";
  cordo::get(a, acc) = 5;
  std::cout << a.x << "\n";
  cordo::get(a, x_field) = 13;
  std::cout << a.x << "\n";

  std::cout << cordo::get(Foo{.x = 6}, acc) << "\n";
  const Foo b = a;
  std::cout << cordo::get(b, acc) << "\n";

  cordo::named_tuple_t<Foo, x_field> desc{};
  cordo::get(a, desc["x"_t]) = 12;
  std::cout << cordo::get(a, acc) << "\n";

  constexpr auto prop_random = cordo::property_(&Foo::random);
  std::cout << cordo::get(a, prop_random) << "\n";
  constexpr auto prop_x_const = cordo::property_(&Foo::x_const);
  std::cout << cordo::get(a, prop_x_const) << "\n";
  constexpr auto prop_x = cordo::property_(&Foo::x_const, &Foo::x_mut);
  cordo::get(a, prop_x) = 16;
  std::cout << cordo::get((const Foo&)a, prop_x) << "\n";

  constexpr auto erased_x = cordo::erased_(cordo::value_t<acc>());
  std::cout << *cordo::get.as<int>(a, erased_x) << " "
            << cordo::get.as<float>(a, erased_x) << "\n";

  constexpr auto composed_z =
      cordo::compose_(cordo::field_(&Baz::l), cordo::field_(&Foo::x));
  Baz baz{};
  cordo::get(baz, composed_z) = 7;
  std::cout << cordo::get((const Baz&)baz, composed_z) << "\n";

  auto ppp = (123_key = 15);
  std::cout << ppp.key()() << " " << ppp.value() << "\n";

  auto sss = ("foo"_key = 16);
  std::cout << sss.key()() << " " << sss.value() << "\n";

  return 0;
}