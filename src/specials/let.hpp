#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    namespace let {
      inline map_t::p bindings(
        const value_t::p& bs, const map_t::p& e, thread_t& t) {

        return imu::reduce([&](const map_t::p& e, const imu::ty::cons& b) {
            auto sym = imu::first<value_t::p>(b);
            auto val = imu::second<value_t::p>(b);
            auto var = imu::nu<var_t>();

            compile(*val, e);
            t << instr::bind << var;

            return imu::assoc(e, *sym, var);
          },
          e,
          imu::partition(2, as<vector_t>(bs)));
      }
    }

    void let_(const list_t::p& forms, const ctx_t& ctx, thread_t& t) {
      auto locals = let::bindings(*imu::first(forms), ctx.env(), t);
      do_(imu::rest(forms), ctx_t(ctx, locals), t);
    }
  }
}
