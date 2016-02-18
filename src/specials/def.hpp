#pragma once

namespace rev {

  namespace specials {

    void def(const list_t::p& forms, const ctx_t& ctx, thread_t& t) {
      auto name = as<sym_t>(imu::first(forms));
      auto init = imu::second(forms);

      if (auto doc = as_nt<string_t>(*init)) {
        init = imu::first(imu::drop(2, forms));
      }

      auto var = imu::nu<var_t>();
      intern(name, var);

      compile(*init, ctx, t);

      t << instr::bind << var;
      t << instr::push << var;
    }
  }
}
