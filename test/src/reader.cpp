#include "reader.hpp"
#include "values.hpp"

#include <cassert>

using namespace rev;

void reader_test_0() {

  auto o   = rdr::read("1");
  auto one = as<int_t>(o);

  assert(one && one->value == 1);
}

void reader_test_1() {

  auto o   = rdr::read("(1 2)");
  auto lst = as<list_t>(o);

  auto fst = as<int_t>(*imu::first(lst));

  assert(fst);
  assert(fst->value == 1);

  auto snd = as<int_t>(*imu::second(lst));

  assert(snd);
  assert(snd->value == 2);
}

void reader_test_2() {

  auto o   = rdr::read("foo");
  auto sym = as<sym_t>(o);

  assert(sym && sym->name() == "foo");

  o   = rdr::read("foo/bar");
  sym = as<sym_t>(o);

  assert(sym);
  assert(sym->name() == "bar");
  assert(sym->ns()   == "foo");
}

void reader_test_3() {

  auto o   = rdr::read("'foo");
  auto lst = as<list_t>(o);

  auto spec =
    list_t::factory(
      sym_t::intern("quote"),
      sym_t::intern("foo"));

  assert(lst == spec);

  o   = rdr::read("`bar");
  lst = as<list_t>(o);

  spec =
    list_t::factory(
      sym_t::intern("syntax-quote"),
      sym_t::intern("bar"));

  assert(lst == spec);
}

void reader_test_4() {

  auto o = rdr::read("[foo bar]");
  auto v = as<vector_t>(o);

  auto fst = as<sym_t>(imu::nth(v, 0));

  assert(fst && fst == sym_t::intern("foo"));

  auto snd = as<sym_t>(imu::nth(v, 1));

  assert(snd && snd == sym_t::intern("bar"));
}

void reader_test_5() {

  auto foo = sym_t::intern("foo");
  auto bar = sym_t::intern("bar");

  auto o = rdr::read("{foo 5 bar 23}");
  auto m = as<map_t>(o);

  assert(m);
  assert(as<int_t>(*imu::get(m, foo))->value == 5);
  assert(as<int_t>(*imu::get(m, bar))->value == 23);
}

void reader_test_6() {

  auto o = rdr::read("\"fruitloops\"");
  auto s = as<string_t>(o);

  assert(s && s->name() == "fruitloops");
}

void reader_test_7() {

  auto o   = rdr::read("@moodswings");
  auto lst = as<list_t>(o);

  auto spec =
    list_t::factory(
      sym_t::intern("deref"),
      sym_t::intern("moodswings"));

  assert(lst == spec);
}

void reader_test_8() {

  auto o   = rdr::read("~moodswings");
  auto lst = as<list_t>(o);

  auto spec =
    list_t::factory(
      sym_t::intern("unquote"),
      sym_t::intern("moodswings"));

  assert(lst == spec);

  o   = rdr::read("~'moodswings");
  lst = as<list_t>(o);

  spec =
    list_t::factory(
      sym_t::intern("quote"),
      sym_t::intern("moodswings"));

  assert(as<sym_t>(*imu::first(lst)) == sym_t::intern("unquote"));
  assert(as<list_t>(*imu::second(lst)) == spec);
}

void reader_test_9() {

  auto o   = rdr::read("(#_1 2)");
  auto lst = as<list_t>(o);

  assert(imu::count(lst) == 1);
  assert(as<int_t>(*imu::first(lst))->value == 2);

  o   = rdr::read("(1 #_2)");
  lst = as<list_t>(o);

  assert(imu::count(lst) == 1);
  assert(as<int_t>(*imu::first(lst))->value == 1);

  assert(rdr::read("#_(1 _2)") == nullptr);
}

int main() {
    reader_test_0();
    reader_test_1();
    reader_test_2();
    reader_test_3();
    reader_test_4();
    reader_test_5();
    reader_test_6();
    reader_test_7();
    reader_test_8();
    reader_test_9();
    std::cout << "All reader tests completed" << std::endl;
}
