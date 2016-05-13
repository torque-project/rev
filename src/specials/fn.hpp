#pragma once

#include "do.hpp"
#include "let.hpp"

namespace rev {

  namespace specials {

    namespace priv {

      typedef std::tuple<bool, int8_t, int64_t> meth_t;
    }

    priv::meth_t body(const list_t::p& meth, ctx_t& ctx, thread_t& t) {

      using namespace priv;

      uint64_t address  = t.size();
      bool     variadic = false;

      auto args   = let::nativize(*imu::first(meth));
      auto body   = imu::rest(meth);
      auto locals = ctx.body();
      auto arity  = 0;

      auto arg = imu::seq(args);
      while (!is_empty(arg)) {
        auto sym = as<sym_t>(imu::first(arg));
        if (sym->name() != "&") {
          locals = locals.local(sym);
          if (!variadic) {
            ++arity;
          }
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

      // TODO: should we support variadic recur?
      auto arglst   = imu::take<list_t::p>(imu::count(args), args);
      auto body_ctx = variadic ? locals : locals.recur(address, arglst);

      // compile body
      do_(body, body_ctx, t);
      t << instr::return_to;

      // return jump point to beginning of body
      auto encoded = fn_t::encode(address, body_ctx.stack_space());
      return meth_t(variadic, arity, encoded);
    }

    void fn(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      using namespace priv;
      static const auto macro = sym_t::intern("macro");

      thread_t thread(8, -1);

      auto name   = as_nt<sym_t>(*imu::first(forms));
      auto fnspec = imu::rest(forms);

      ctx_t fn_ctx = name ? ctx.closure(name) : ctx.closure();

      if (!name) {
        name   = sym_t::intern("anon");
        fnspec = forms;
      }

      if (protocol_t::satisfies(protocol_t::ivector, *imu::first(fnspec))) {
        fnspec = list_t::factory(fnspec);
      }

      bool    variadic;
      int8_t  arity;
      int8_t  max_arity      = -1;
      int8_t  variadic_arity = -1;
      int64_t off;

      try {
        imu::for_each([&](const list_t::p& meth) {
          std::tie(variadic, arity, off) = body(meth, fn_ctx, thread);

          // TODO: check for duplicate variadic bodies
          variadic_arity = variadic ? arity : variadic_arity;
          max_arity      = variadic ? max_arity : std::max(arity, max_arity);
          thread[variadic ? (variadic_arity+1) : arity] = off;
        },
        fnspec);
      }
      catch(...) {
        std::cout
          << "While compiling fn: " << name->name()
          << std::endl << "  ";
        throw;
      }

      auto address   = finalize_thread(thread);
      auto nenclosed = compile_all(fn_ctx.closed_overs(), ctx, t);
      auto is_macro  = imu::get(as<map_t>(name->meta), macro, sym_t::false_);
      auto arities   = (int64_t) ((uint8_t) max_arity) |
        (((uint8_t) variadic_arity) << 8);

      t << instr::closure << name << address << nenclosed << arities;
    }
  }
}
