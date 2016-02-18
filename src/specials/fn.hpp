#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    fn_t::meth_t body(const list_t::p& meth, const ctx_t& ctx, thread_t& t) {

      uint64_t address = t.size();
      auto args = as<vector_t>(imu::first(meth));

      auto locals = imu::reduce(
        [&](const map_t::p& m, const sym_t::p& x) {

          auto var = imu::nu<var_t>();
          t << instr::bind << var;

          return imu::assoc(m, x, var);
        },
        imu::nu<map_t>(),
        args);

      // compile body
      do_(imu::rest(meth), ctx_t(ctx, locals, address), t);
      // jump to return address
      t << instr::return_to;

      // return jump point to begin of body
      return fn_t::meth_t(address, imu::count(args));
    }

    void fn(const list_t::p& forms, const ctx_t& ctx, thread_t& t) {

      thread_t thread;

      auto name   = as_nt<sym_t>(*imu::first(forms));
      auto fnspec = imu::rest(forms);

      if (!name) {
        name   = sym_t::intern("lamba");
        fnspec = forms;
      }

      if (is<vector_t>(*imu::first(fnspec))) {
        fnspec = list_t::factory(fnspec);
      }

      auto meths = imu::map([&](const list_t::p& meth) {
        return body(meth, ctx, thread);
      }, fnspec);

      // copy function to main thread and obtain final code address
      auto final_address = finalize_thread(thread);

      // update jump addresses with final positions in main thread
      meths = imu::map([&](const fn_t::meth_t& meth) {
        return fn_t::meth_t(final_address + meth.address, meth.arity);
      }, meths);

      // push fn onto stack in calling thread
      t << instr::push << imu::nu<fn_t>(name->name(), meths);
    }
  }
}
