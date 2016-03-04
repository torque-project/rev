#pragma once

#include "do.hpp"

namespace rev {

  namespace specials {

    void defprotocol(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      static sym_t::p FN       = sym_t::intern("fn*");
      static sym_t::p DISPATCH = sym_t::intern("dispatch*");
      static sym_t::p DEF      = sym_t::intern("def");

      // build meta information about the protocol, which
      // gets used later while compiling deftypes that
      // use this protocol

      auto name     = as<sym_t>(imu::first(forms));
      auto protocol = imu::nu<protocol_t>(name->name(), imu::rest(forms));
      auto var      = imu::nu<var_t>();

      var->bind(protocol);
      intern(name, var);

      // this expands a method definition:
      // (-meth [this a b])
      // into a function:
      // (def -meth (fn [this a b] (dispatch* <proto> this a b)))

      int64_t n = 0;

      auto fns = imu::map<list_t::p>([&](const value_t::p& v) {
        auto lst  = as<list_t>(v);
        auto meth = as<sym_t>(imu::first(lst));
        auto id   = n++;
        return list_t::factory(
          DEF, meth,
          imu::conj(
            imu::map<list_t::p>([&](const value_t::p& arglst) {
              auto v    = as<vector_t>(arglst);
              auto args = into(imu::nu<list_t>(), imu::seq(v));
              auto body = imu::conj(
                            imu::conj(
                              imu::conj(args, imu::nu<int_t>(id)),
                              name),
                            DISPATCH);
              return list_t::factory(v, body);
            }, imu::rest(lst)),
            FN));
      },
      imu::rest(forms));

      do_(fns, ctx, t);

      t << instr::pop << instr::push << var;
    }

    void dispatch(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto proto = as<sym_t>(imu::first(forms));
      auto meth  = as<int_t>(imu::second(forms));
      auto args  = imu::drop(2, forms);
      compile_all(args, ctx, t);
      compile(proto, ctx, t);
      t << instr::method << meth->value << imu::count(args);
    }
  }
}
