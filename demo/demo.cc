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

  constexpr auto x_acc = cordo::field(&Foo::x);
  constexpr auto x_field = cordo::named("x"_key, cordo::field(&Foo::x));

  constexpr auto kv = ("x"_key = x_acc);

  Foo a{.x = 3};
  std::cout << a.x << "\n";
  cordo::get(x_acc, a) = 5;
  std::cout << a.x << "\n";
  cordo::get(x_field, a) = 13;
  std::cout << a.x << "\n";

  const Foo b = a;
  std::cout << cordo::get(x_acc, b) << "\n";

  std::cout << cordo::get.as<int>(x_acc, b) << "\n";

  cordo::struct_<"Foo"_key, Foo, kv, (0_key = x_acc)> mirror{};
  cordo::get(mirror["x"_key], a) = 12;
  std::cout << cordo::get(mirror[0_key], a) << "\n";

  constexpr auto prop_random = cordo::property(&Foo::random);
  std::cout << cordo::get(prop_random, a) << "\n";
  constexpr auto prop_x_const = cordo::property(&Foo::x_const);
  std::cout << cordo::get(prop_x_const, a) << "\n";
  constexpr auto prop_x = cordo::property(&Foo::x_const, &Foo::x_mut);
  cordo::get(prop_x, a) = 16;
  std::cout << cordo::get(prop_x, (const Foo&)a) << "\n";

  constexpr auto erased_x = cordo::erased(cordo::value_t<x_acc>());
  std::cout << *cordo::get.as<int>(erased_x, a) << " "
            << cordo::get.as<float>(erased_x, a) << "\n";

  constexpr auto composed_z =
      cordo::compose(cordo::field(&Baz::l), cordo::field(&Foo::x));
  Baz baz{};
  cordo::get(composed_z, baz) = 7;
  std::cout << cordo::get(composed_z, (const Baz&)baz) << "\n";

  auto ppp = (123_key = 15);
  std::cout << ppp.key()()() << " " << ppp.value() << "\n";

  auto sss = ("foo"_key = 16);
  std::cout << sss.key()()() << " " << sss.value() << "\n";

  std::cout << (cordo::get(x_acc, a) = 89) << "\n";

  struct Noisy {
    int x = 7;
    ~Noisy() { std::cout << "loud\n"; }
  };
  std::cout << cordo::any_v(Noisy{}).as<Noisy>()->x << "\n";

  cordo::any<cordo::any_v(cordo::get_cpo, const Foo&)> erased_acc{x_acc};
  std::cout << *erased_acc.invoke(cordo::get_cpo{}, a).as<int>() << "\n";
  return 0;
}