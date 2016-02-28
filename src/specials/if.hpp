#pragma once

#include "../instructions.hpp"

namespace rev {

  namespace specials {

    void if_(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto cond  = imu::first(forms);
      auto then  = imu::second(forms);
      auto else_ = imu::first(imu::drop(2, forms));

      compile(*cond, ctx, t);
      t << instr::brcond << 0;
      auto eip = t.size();

      if (then) {
        compile(*then, ctx, t);
      }
      else {
        t << instr::push << nullptr;
      }

      t << instr::br << 0;
      auto cont = t.size();

      t[eip-1] = (t.size() - eip);
      compile(*else_, ctx, t);

      t[cont-1] = (t.size() - cont) + 1;
    }
  }
}
