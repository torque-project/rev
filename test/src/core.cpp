#include "core.hpp"
#include "values.hpp"

#include <cassert>

using namespace rev;

void core_test_0() {
  auto o = read("(torque.core.builtin/+ 1 1)");
  auto n = eval(o);

  assert(as<int_t>(n)->value == 2);
}

void core_test_1() {
  auto o = read("(torque.core.builtin/- (torque.core.builtin// (torque.core.builtin/* 2 2) 2) 1)");
  auto n = eval(o);

  assert(as<int_t>(n)->value == 1);

  o = read("(torque.core.builtin/- 4 2)");
  n = eval(o);

  assert(as<int_t>(n)->value == 2);

  o = read("(torque.core.builtin/- 2 4)");
  n = eval(o);

  assert(as<int_t>(n)->value == -2);
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

void core_test_3() {

  auto o = read("(if (torque.core.builtin/< 1 2) 8 5)");
  auto n = eval(o);

  assert(as<int_t>(n)->value == 8);

  o = read("(if (torque.core.builtin/> 1 2) 8 5)");
  n = eval(o);

  assert(as<int_t>(n)->value == 5);
}

void core_test_4() {
  auto o = read("(do 1 2 3)");
  auto n = eval(o);

  assert(as<int_t>(n)->value == 3);
}

void core_test_5() {
  auto o = read("(let* [x 1] (torque.core.builtin/+ x 1))");
  auto n = eval(o);

  assert(as<int_t>(n)->value == 2);
}

void core_test_6() {
  auto o = read("(def x 5)");
  auto p = read("(torque.core.builtin/+ x 6)");
  eval(o);
  auto n = eval(p);

  assert(as<int_t>(n)->value == 11);
}

void core_test_7() {
  auto o = read(
    "(loop* [x 1]" \
      "(if (torque.core.builtin/< x 5)" \
        "(recur (torque.core.builtin/+ x 1))" \
        "x)))");
  auto n = eval(o);

  assert(as<int_t>(n)->value == 5);
}

void core_test_8() {

  auto o = read("(def inc (fn* [x] (torque.core.builtin/+ x 21)))");
  auto p = read("(inc 1)");
  eval(o);
  auto n = eval(p);

  assert(as<int_t>(n)->value == 22);

  o = read("((fn* [x] (torque.core.builtin/+ x 1)) 88)");
  n = eval(o);

  assert(as<int_t>(n)->value == 89);

  o = read(
    "(let* [f ((fn* [x] (fn* [y] (torque.core.builtin/+ x y))) 18)]" \
      "(f 1))");
  n = eval(o);

  assert(as<int_t>(n)->value == 19);

  o = read(
    "(let* [f (fn* [x] (fn* [y] (torque.core.builtin/+ x y)))" \
           "a (f 1)" \
           "b (f 8)]" \
      "(a 1)" \
      "(b 2))");
  n = eval(o);

  assert(as<int_t>(n)->value == 10);

  o = read(
    "(let* [f (fn* [x] (fn* [y] (torque.core.builtin/+ x y)))" \
           "a (f 1)" \
           "b (f 8)" \
           "x (a 1)" \
           "y (b 2)]"
      "x)");
  n = eval(o);

  assert(as<int_t>(n)->value == 2);

  o = read(
    "(let* [f (fn* [x] (fn* [y] (fn* [] (torque.core.builtin/+ x y))))" \
           "g (f 7)"
           "h (g 6)]" \
      "(h))");
  n = eval(o);

  assert(as<int_t>(n)->value == 13);
}

void core_test_9() {

  auto o = read("(def ^:macro reverse (fn* [a b c] '(~c ~b ~a)))");
  auto p = read("(reverse 5 8 torque.core.builtin/-)");
  eval(o);
  auto n = eval(p);

  assert(as<int_t>(n)->value == 3);
}

int main() {
  rev::boot(1 << 16);
  core_test_0();
  core_test_1();
  core_test_2();
  core_test_3();
  core_test_4();
  core_test_5();
  core_test_6();
  core_test_7();
  core_test_8();
  core_test_9();
  std::cout << "All core tests completed" << std::endl;
}
