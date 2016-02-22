#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    namespace priv {

      typedef std::tuple<uint8_t, int64_t> meth_t;
    }

    priv::meth_t body(const list_t::p& meth, ctx_t& ctx, thread_t& t) {

      using namespace priv;

      uint64_t address = t.size();

      auto args = as<vector_t>(imu::first(meth));
      auto body = imu::rest(meth);

      auto locals = imu::reduce(
        [&](const map_t::p& m, const sym_t::p& x) {

          auto var = imu::nu<var_t>();
          t << instr::bind << var;

          return imu::assoc(m, x, var);
        },
        imu::nu<map_t>(),
        args);

      // compile body
      auto body_ctx = ctx.recur(locals, address);
      do_(body, body_ctx, t);
      t << instr::return_to;

      // return jump point to beginning of body
      return meth_t(imu::count(args), address);
    }

    void fn(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      using namespace priv;

      thread_t thread(8, -1);
      ctx_t    fn_ctx = ctx.fn();

      auto name   = as_nt<sym_t>(*imu::first(forms));
      auto fnspec = imu::rest(forms);

      if (!name) {
        name   = sym_t::intern("lamba");
        fnspec = forms;
      }

      if (is<vector_t>(*imu::first(fnspec))) {
        fnspec = list_t::factory(fnspec);
      }

      uint8_t arity;
      int64_t off;

      imu::for_each([&](const list_t::p& meth) {
        std::tie(arity, off) = body(meth, fn_ctx, thread);
        thread[arity] = off;
      },
      fnspec);

      auto address = finalize_thread(thread);

      // push closed overs onto the stack by compiling their
      // symbol in this fns parent context (i.e. the enclosing fn).
      // this will also propagate the closure to the parent fn if
      // neccessary
      auto nenclosed =
        imu::reduce([&](int n, const sym_t::p& sym) {
          compile(sym, ctx, t);
          return n + 1;
        },
        0,
        fn_ctx.closed_overs());

      t << instr::closure << address << nenclosed;
    }
  }
}
