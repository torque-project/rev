#pragma once

namespace rev {

  namespace builtins {

    using namespace instr::stack;

    void add(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "add" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = imu::nu<int_t>(a->value + b->value);
      push(s, r);
    }

    void sub(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "sub" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = imu::nu<int_t>(a->value - b->value);
      push(s, r);
    }

    void mul(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "mul" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = imu::nu<int_t>(a->value * b->value);
      push(s, r);
    }

    void div(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "div" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = imu::nu<int_t>(a->value / b->value);
      push(s, r);
    }

    void lt(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "lt" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = (a->value < b->value ? sym_t::true_ : sym_t::false_);
      push(s, r);
    }

    void gt(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "gt" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = (a->value > b->value ? sym_t::true_ : sym_t::false_);
      push(s, r);
    }

    void lte(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "lte" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = (a->value <= b->value ? sym_t::true_ : sym_t::false_);
      push(s, r);
    }

    void gte(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "gte" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = (a->value >= b->value ? sym_t::true_ : sym_t::false_);
      push(s, r);
    }

    void eq(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "eq" << std::endl;
#endif
      auto b = as<int_t>(pop<value_t::p>(s));
      auto a = as<int_t>(pop<value_t::p>(s));
      auto r = (a->value == b->value ? sym_t::true_ : sym_t::false_);
      push(s, r);
    }
  }
}
