#pragma once

namespace rev {

  namespace specials {

    void quote(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      t << instr::push << *imu::first(forms);
    }
  }
}
