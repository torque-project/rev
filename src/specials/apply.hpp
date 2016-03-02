#pragma once

namespace rev {

  namespace specials {

    void apply(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      auto f    = imu::first(forms);
      auto args = imu::second(forms);

      compile(*f, ctx, t);
      compile(*args, ctx, t);

      t << instr::apply;
    }
  }
}
