#pragma once

namespace rev {
  namespace specials {
    void throw_(const list_t::p& form, ctx_t& ctx, thread_t& t) {
      compile(*imu::first(form), ctx, t);
      t << instr::throw_;
    }
  }
}
