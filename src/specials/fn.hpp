#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    namespace priv {

      typedef std::tuple<bool, uint8_t, int64_t> meth_t;
    }

    priv::meth_t body(const list_t::p& meth, ctx_t& ctx, thread_t& t) {

      using namespace priv;

      uint64_t address  = t.size();
      bool     variadic = false;

      auto args   = as<vector_t>(imu::first(meth));
      auto body   = imu::rest(meth);
      auto locals = imu::nu<map_t>();
      auto arity  = 0;

      auto arg = imu::seq(args);
      while (!is_empty(arg)) {
        auto sym = as<sym_t>(imu::first(arg));
        if (sym->name() != "&") {
          auto var = imu::nu<var_t>();
          t << instr::bind << var;

          locals = imu::assoc(locals, sym, var);
          ++arity;
        }
        else {
          if (variadic) {
            throw std::runtime_error("Only one rest arg symbol allowed");
          }
          variadic = true;
          if (imu::is_empty(arg)) {
            throw std::runtime_error("Expecting symbol after &");
          }
        }
        arg = imu::rest(arg);
      }

      // compile body
      auto body_ctx = ctx.recur(locals, address);
      do_(body, body_ctx, t);
      t << instr::return_to;

      // return jump point to beginning of body
      return meth_t(variadic, arity, address);
    }

    void fn(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      using namespace priv;
      static const auto macro = sym_t::intern("macro");

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

      bool    variadic;
      uint8_t arity;
      uint8_t max_arity = 0;
      int64_t off;

      imu::for_each([&](const list_t::p& meth) {
        std::tie(variadic, arity, off) = body(meth, fn_ctx, thread);

        // TODO: check for duplicate variadic bodies

        max_arity = variadic ? max_arity : std::max(arity, max_arity);
        thread[arity] = off;
      },
      fnspec);

      auto address   = finalize_thread(thread);
      auto nenclosed = compile_all(fn_ctx.closed_overs(), ctx, t);
      auto is_macro  = imu::get(as<map_t>(name->meta), macro, sym_t::false_);

      t << instr::closure << address << nenclosed << max_arity;
    }
  }
}
