#include <vector>
namespace foo {
struct __attribute__((annotate("cordo"))) Foo final {
  int bar;
  std::vector<int> baz;
};
}  // namespace foo