#pragma once

#include "../instructions.hpp"

namespace rev {

  namespace specials {

    void if_(const list_t::p& forms, const ctx_t& ctx, thread_t& t) {
      auto cond  = imu::first(forms);
      auto then  = imu::second(forms);
      auto else_ = imu::first(imu::drop(2, forms));

      compile(*cond, ctx);
      t << instr::brcond;
      auto eip = t.insert(t.end(), nullptr);

      compile(*then, ctx);
      t << instr::brrel;
      auto cont = t.insert(t.end(), nullptr);

      *eip = (void*) (t.end() - eip);
      compile(*else_, ctx);

      *cont = (void*) (t.end() - cont);
    }
  }
}
