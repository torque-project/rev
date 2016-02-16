#pragma once

#include "../instructions.hpp"

namespace rev {

  namespace specials {

    inline void if_(const list_t::p& forms, const map_t::p& e, thread_t& t) {
      auto cond  = imu::first(forms);
      auto then  = imu::second(forms);
      auto else_ = imu::first(imu::drop(2, forms));

      compile(*cond, e);
      t << instr::brcond;
      auto eip = t.insert(t.end(), nullptr);

      compile(*then, e);
      t << instr::brrel;
      auto cont = t.insert(t.end(), nullptr);

      *eip = (void*) (t.end() - eip);
      compile(*else_, e);

      *cont = (void*) (t.end() - cont);
    }
  }
}
