#include "core.hpp"
#include "values.hpp"

#include <cassert>

using namespace rev;

void core_test_0() {
  auto o = read("(torque.core.builtin/+ 1 1)");
  auto n = eval(o);

  assert(as<int_t>(n)->value = 2);
}

void core_test_1() {
  auto o = read("(torque.core.builtin/- (torque.core.builtin// (torque.core.builtin/* 2 2) 2) 1)");
  auto n = eval(o);

  assert(as<int_t>(n)->value = 1);

  o = read("(torque.core.builtin/- 4 2)");
  n = eval(o);

  assert(as<int_t>(n)->value = 2);

  o = read("(torque.core.builtin/- 2 4)");
  n = eval(o);

  assert(as<int_t>(n)->value = -2);
}

void core_test_2() {
  auto o = read("(torque.core.builtin/< 1 2)");
  auto b = eval(o);

  assert(is_true(b));

  o = read("(torque.core.builtin/> 4 1)");
  b = eval(o);

  assert(is_true(b));

  o = read("(torque.core.builtin/<= 3 3)");
  b = eval(o);

  assert(is_true(b));

  o = read("(torque.core.builtin/== 3 3)");
  b = eval(o);

  assert(is_true(b));

  o = read("(torque.core.builtin/>= 3 3)");
  b = eval(o);

  assert(is_true(b));

  o = read("(torque.core.builtin/>= 5 2)");
  b = eval(o);

  assert(is_true(b));

  o = read("(torque.core.builtin/<= 8 22)");
  b = eval(o);

  assert(is_true(b));

  o = read("(torque.core.builtin/== 12 9)");
  b = eval(o);

  assert(is_false(b));
}

int main() {
  rev::boot();
  core_test_0();
  core_test_1();
  core_test_2();
  std::cout << "All core tests completed" << std::endl;
}
