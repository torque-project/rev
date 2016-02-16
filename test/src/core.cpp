#include "core.hpp"
#include "values.hpp"

#include <cassert>

using namespace rev;

void core_test_0() {
  auto o = read("(torque.core.builtin/+ 1 1)");
  auto n = eval(o);

  assert(as<int_t>(n)->value = 2);
}

int main() {
  rev::boot();
  core_test_0();
  std::cout << "All core tests completed" << std::endl;
}
