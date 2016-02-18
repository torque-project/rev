#pragma once

#include <cassert>

namespace rev {

  namespace specials {

    void do_(const list_t::p& forms, const ctx_t& ctx, thread_t& t) {

      if (imu::is_empty(forms)) {
        t << instr::push << nullptr;
      }
      else {
        auto form = imu::first(forms);
        auto rest = imu::rest(forms);

        while (!imu::is_empty(rest)) {
          compile(*form, ctx, t);
          t << instr::pop;

          form = imu::first(rest);
          rest = imu::rest(rest);
        }

        assert(form && "Expect do to have a return form");
        compile(*form, ctx, t);
      }
    }
  }
}
