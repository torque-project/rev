#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    namespace let {
      inline ctx_t bindings(const vector_t::p& bs, ctx_t& ctx, thread_t& t) {

        return imu::reduce([&](ctx_t& ctx, const list_t::p& b) {

            auto sym   = as<sym_t>(imu::first(b));
            auto val   = imu::second(b);
            auto local = ctx.local(sym);

            compile(*val, local, t);
            t << instr::assign << as<int_t>(local[sym])->value;

            return local;
          },
          ctx,
          imu::partition<list_t::p>(2, bs));
      }
    }

    void let_(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto locals = let::bindings(as<vector_t>(imu::first(forms)), ctx, t);
      do_(imu::rest(forms), locals, t);
    }
  }
}
