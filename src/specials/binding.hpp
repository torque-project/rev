#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    void binding(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      auto bindings = imu::seq(let::nativize(*imu::first(forms)));
      auto syms     = imu::take_nth<list_t::p>(2, bindings);
      auto vals     = imu::take_nth<list_t::p>(2, imu::drop(1, bindings));

      imu::for_each([&](const value_t::p& b) {
          compile(b, ctx, t);
        }, vals);

      auto vars = imu::map<list_t::p>([&](const sym_t::p& sym) {
          return *resolve(ctx, sym);
        }, imu::into<list_t::p>(imu::nu<list_t>(), syms));

      t << instr::push_bind << vars;
      do_(imu::rest(forms), ctx, t);
      t << instr::pop_bind << vars;
    }
  }
}
