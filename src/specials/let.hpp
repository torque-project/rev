#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    namespace let {
      inline map_t::p bindings(const value_t::p& bs, ctx_t& ctx, thread_t& t) {

        return imu::reduce([&](const map_t::p& m, const imu::ty::cons& b) {

            auto sym      = imu::first<value_t::p>(b);
            auto val      = imu::second<value_t::p>(b);
            auto var      = imu::nu<var_t>();
            auto bindings = imu::assoc(m, *sym, var);
            auto c        = ctx.local(bindings);

            compile(*val, c, t);
            t << instr::bind << var;

            return bindings;
          },
          imu::nu<map_t>(),
          imu::partition(2, as<vector_t>(bs)));
      }
    }

    void let_(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto locals = ctx.local(let::bindings(*imu::first(forms), ctx, t));
      do_(imu::rest(forms), locals, t);
    }
  }
}
