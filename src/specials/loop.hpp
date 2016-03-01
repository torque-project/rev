#pragma once

#include "do.hpp"
#include "let.hpp"

namespace rev {

  namespace specials {

    void loop(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto bindings = as<vector_t>(imu::first(forms));
      auto locals   = let::bindings(bindings, ctx, t);
      auto point    = (int64_t) t.size();
      auto recur    = locals.recur(point, imu::take_nth<list_t::p>(2, bindings));
      do_(imu::rest(forms), recur, t);
    }

    void recur(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      compile_all(forms, ctx, t);

      imu::for_each([&](const sym_t::p& sym) {
          if (auto local = ctx[sym]) {
            t << instr::assign << as<int_t>(*local)->value;
          }
          else {
            throw std::runtime_error(
              "Can't find recur binding: " + sym->name());
          }
        }, imu::into(imu::nu<list_t>(), ctx.recur_syms()));

      t << instr::br << (ctx.recur_to() - ((int64_t) t.size()));
    }
  }
}
