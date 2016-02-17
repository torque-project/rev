#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    void let(const list_t::p& forms, const map_t::p& env, thread_t& t) {

      // bind new local vars
      auto bindings =
        imu::reduce([&](const map_t::p& e, const imu::ty::cons& binding) {

            auto sym = imu::first<value_t::p>(binding);
            auto val = imu::second<value_t::p>(binding);
            auto var = imu::nu<var_t>();

            compile(*val, e);
            t << instr::bind << var;

            return imu::assoc(env, *sym, var);
          },
          env,
          imu::partition(2, as<vector_t>(imu::first(forms))));

      // treat the rest of the forms in the let as if they are wrapped in
      // an implicit 'do' form
      do_(imu::rest(forms), bindings, t);
    }
  }
}
