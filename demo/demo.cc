#include <cordo/impl/core/anno.hh>
#include <cordo/impl/struct.hh>
#include <iostream>

#include "cordo/cordo.hh"
// #include "demo.cordo.hh"
using namespace ::cordo::literals;

struct Foo {
  int x;

  int random() const { return 6; }
  const int& x_const() const { return x; }
  int& x_mut() { return x; }
};

struct Baz {
  Foo l;
};

constexpr auto cordo_cpo(cordo::mirror_traits_cpo, cordo::tag_t<Foo>) noexcept {
  return cordo::struct_<"Foo"_key, Foo, ("x"_key = &Foo::x),
                        (0_key = &Foo::x)>{};
}
constexpr auto cordo_cpo(cordo::mirror_traits_cpo,
                         cordo::tag_t<const Foo>) noexcept {
  return cordo::struct_<"Foo"_key, Foo, ("x"_key = &Foo::x),
                        (0_key = &Foo::x)>{};
}

void anno_demo() {
  ::cordo::anno(::cordo_internal_struct::field_anno_sema{}, 0_key <= &Foo::x);
}

void mirror_demo() {
  Foo f{.x = 3};
  auto mf = cordo::mirror(f);

  std::cout << mf[0_key].v() << " " << mf[cordo::make_key<(size_t)0>()].v()
            << "\n";
  mf["x"_key] = 16;
  std::cout << mf["x"_key].v() << " " << f.x << "\n";

  const Foo& cf = f;
  auto cmf = cordo::mirror(cf);
  std::cout << cmf[0_key].v() << "\n";
}

void kv_demo() {
  // This produces the inversed list!
  // TODO: To be used with flatten fields.
  cordo::kv_t kv = (99_key <= &Baz::l || &Foo::x || 3 || Foo{7});
  std::cout << kv.key()() << " " << kv.value().size() << " " << kv.value().h.x
            << " " << kv.value().t.h << "\n";
}

int main(int argc, const char** argv) {
  mirror_demo();
  kv_demo();

  constexpr auto x_acc = cordo::field(&Foo::x);
  constexpr auto kv = ("x"_key = x_acc);
  Foo a{.x = 3};
  std::cout << a.x << "\n";
  cordo::get(x_acc, a) = 5;
  std::cout << a.x << "\n";

  const Foo b = a;
  std::cout << cordo::get(x_acc, b) << "\n";

  using vt = cordo::values_t<("foo"_key = 44), (2_key = 56)>;
  std::cout << cordo::kv_lookup(vt{}, 2_key) << "\n";
  std::cout << cordo::kv_lookup(vt{}, "foo"_key) << "\n";

  // cordo::struct_<"Foo"_key, Foo, kv, (0_key = &Foo::x)> mirror{};
  // ::cordo::get(s, ::cordo::make_accessor( ::cordo::kv_lookup(typename
  // decltype(mirror)::fields_t{}, k))))
  // cordo::invoke(cordo::mirror_subscript_cpo{}, a, mirror, "x"_key) = 12;
  cordo::struct_ mt =
      cordo::invoke(cordo::mirror_traits_cpo{}, cordo::tag_t<Foo>());
  static_assert(::cordo_internal_struct::struct_meta<decltype(mt)>);
  // cordo_cpo(cordo::mirror_fn_cpo{}, ::cordo_internal_cpo::adl_tag{}, mt, a);
  // cordo::invoke(cordo::mirror_fn_cpo{}, mt, a);
  // auto m = cordo::mirror(a);
  // std::cout << m["x"_key] << "\n";
  // m["x"_key] = 239;
  // std::cout << m[0_key] << "\n";
  // std::cout << cordo::invoke(cordo::mirror_subscript_cpo{}, a, mirror, 0_key)
  // << "\n";

  constexpr auto prop_random = cordo::property(&Foo::random);
  std::cout << cordo::get(prop_random, a) << "\n";
  constexpr auto prop_x_const = cordo::property(&Foo::x_const);
  std::cout << cordo::get(prop_x_const, a) << "\n";
  constexpr auto prop_x = cordo::property(&Foo::x_const, &Foo::x_mut);
  cordo::get(prop_x, a) = 16;
  std::cout << cordo::get(prop_x, (const Foo&)a) << "\n";

  constexpr auto composed_z =
      cordo::compose(cordo::field(&Baz::l), cordo::field(&Foo::x));
  Baz baz{};
  cordo::get(composed_z, baz) = 7;
  std::cout << cordo::get(composed_z, (const Baz&)baz) << "\n";

  auto ppp = (123_key = 15);
  std::cout << ppp.key()() << " " << ppp.value() << "\n";

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