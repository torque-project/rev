#pragma once

namespace rev {

  namespace builtins {

    void add(stack_t& s, thread_t::iterator& ip) {
      auto a = as<int_t>(s.top()); s.pop();
      auto b = as<int_t>(s.top()); s.pop();
      auto r = imu::nu<int_t>(a->value + b->value);
      s.push(r);
    }
  }
}
