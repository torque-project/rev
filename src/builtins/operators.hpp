#pragma once

namespace rev {

  namespace builtins {

    void add(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = imu::nu<int_t>(a->value + b->value);
      s.push(r);
    }

    void sub(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = imu::nu<int_t>(a->value - b->value);
      s.push(r);
    }

    void mul(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = imu::nu<int_t>(a->value * b->value);
      s.push(r);
    }

    void div(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = imu::nu<int_t>(a->value / b->value);
      s.push(r);
    }

    void lt(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = (a->value < b->value ? sym_t::true_ : sym_t::false_);
      s.push(r);
    }

    void gt(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = (a->value > b->value ? sym_t::true_ : sym_t::false_);
      s.push(r);
    }

    void lte(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = (a->value <= b->value ? sym_t::true_ : sym_t::false_);
      s.push(r);
    }

    void gte(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = (a->value >= b->value ? sym_t::true_ : sym_t::false_);
      s.push(r);
    }

    void eq(stack_t& s, thread_t::iterator& ip) {
      auto b = as<int_t>(s.top()); s.pop();
      auto a = as<int_t>(s.top()); s.pop();
      auto r = (a->value == b->value ? sym_t::true_ : sym_t::false_);
      s.push(r);
    }
  }
}
