#pragma once

#include "do.hpp"
#include "let.hpp"

namespace rev {

  namespace specials {

    void loop(const list_t::p& forms, const ctx_t& ctx, thread_t& t) {
      auto locals = let::bindings(*imu::first(forms), ctx.env(), t);
      auto point  = (int64_t) t.size();
      do_(imu::rest(forms), ctx_t(ctx, locals, point), t);
    }

    void recur(const list_t::p& forms, const ctx_t& ctx, thread_t& t) {

      compile_all(forms, ctx, t);

      auto syms = ctx.recur_syms();

      while (auto sym = imu::first(syms)) {
        if (auto var = ctx[as<sym_t>(sym)]) {
          t << instr::bind << *var;
        }
        else {
          throw std::runtime_error(
            "Unbound recur bindind: " + as<sym_t>(sym)->name());
        }
        syms = imu::rest(syms);
      }

      t << instr::br << (ctx.recur_to() - ((int64_t) t.size()));
    }
  }
}
