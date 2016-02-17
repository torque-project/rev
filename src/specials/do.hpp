#pragma once

#include <cassert>

namespace rev {

  namespace specials {

    void do_(const list_t::p& forms, const map_t::p& env, thread_t& t) {

      if (imu::is_empty(forms)) {
        t << instr::push << nullptr;
      }
      else {
        auto form = imu::first(forms);
        auto rest = imu::rest(forms);

        while (!imu::is_empty(rest)) {
          std::cout << "instr" << std::endl;
          compile(*form, env);
          t << instr::pop;

          form = imu::first(rest);
          rest = imu::rest(rest);
        }

        assert(form && "Expect do to have a return form");
        compile(*form, env);
      }
    }
  }
}
