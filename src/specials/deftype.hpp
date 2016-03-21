#pragma once

#include "fn.hpp"

namespace rev {

  namespace specials {

    decltype(auto) build_proto_info(const list_t::p& forms) {
      return imu::reduce(
        [&](const imu::ty::array_map::p& m, const list_t::p& meth) {
          auto name  = *imu::first<sym_t::p>(meth);
          auto arity = imu::count(*imu::second<vector_t::p>(meth));
          auto body  = imu::rest(meth);
          return imu::update(m, name,
            [&](const imu::ty::array_map::p& impls) {
              return imu::assoc(impls, arity, body);
            });
        },
        imu::array_map(),
        forms);
    }

    decltype(auto) build_meth_info(const list_t::p& forms) {

      auto by_proto = imu::partition_by<list_t::p>(&is_symbol, forms);

      return imu::reduce([&](const imu::ty::array_map::p& m, const list_t::p& v) {
          // auto proto = as<sym_t>(imu::ffirst<list_t>(v));
          auto proto    = *imu::first<sym_t::p>(as<list_t>(imu::first(v)));
          auto meths    = as<list_t>(imu::second(v));
          auto by_arity = build_proto_info(meths);
          return assoc(m, proto, by_arity);
        },
        imu::array_map(),
        imu::partition<list_t::p>(2, by_proto));
    }

    decltype(auto) arities(const list_t::p& meth) {
      return imu::map([](const vector_t::p& v) {
          return imu::count(v);
        }, imu::rest(meth));
    }

    void emit_type_impls(type_t::p t, imu::ty::array_map::p& impls, ctx_t& ctx) {

      thread_t thread;

      bool    variadic;
      uint8_t arity;
      int64_t off;

      t->_methods = new type_t::ext_t[imu::count(impls)];

      int64_t n = 0;
      imu::for_each([&](const imu::ty::array_map::value_type& kv) {

        auto name  = imu::first<sym_t::p>(kv);
        auto proto = as<var_t>(*resolve(ctx, name));
        auto impls = imu::second<imu::ty::array_map::p>(kv);

        t->_methods[n].id    = proto->deref<protocol_t>()->_id;
        t->_methods[n].impls = new type_t::impl_t[imu::count(impls)];
        auto& ext = t->_methods[n++];

        int64_t m = 0;
        imu::for_each([&](const list_t::p& meth) {

          auto& native = ext.impls[m++];
          auto name    = as<sym_t>(imu::first(meth));
          auto arities = specials::arities(meth);

          memset(native.arities, 0, sizeof(native.arities));

          if (auto impl = imu::get<imu::ty::array_map::p>(impls, name)) {

            imu::for_each([&](uint64_t arity) {

              if (auto b = imu::get<list_t::p>(*impl, arity)) {
                std::tie(variadic, arity, off) = body(*b, ctx, thread);
              }
#ifdef _DEBUG
              else {
                std::cout
                  << "Type " << t->name() << " doesn't implement arity: "
                  << arity << " of method: "<< name->name()
                  << std::endl;
              }
#endif
              if (variadic) {
                throw std::runtime_error(
                  "Protocol methods can not be variadic");
              }

              native.arities[arity] = t->prepare_closure(arity, off);

            }, arities);
          }
#ifdef _DEBUG
          else {
            std::cout
              << "Type " << t->name() << " doesn't implement method: "
              << name->name() << std::endl;
          }
#endif
        }, proto->deref<protocol_t>()->meths());
      }, impls);

      t->_num_ext = n;
      t->finalize(finalize_thread(thread));
    }

    void deftype(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto name   = as<sym_t>(imu::first(forms));
      auto fields = as<vector_t>(imu::second(forms));
      auto impls  = build_meth_info(imu::drop(2, forms));
      auto var    = imu::nu<var_t>();
      auto type   = imu::nu<type_value_t>(name->name(), fields);

      var->bind(type);
      intern(name, var);

      auto type_ctx = imu::reduce([&](ctx_t& ctx, const sym_t::p& sym) {
          return ctx.local(sym);
        }, ctx, fields);

      type_ctx = type_ctx.closure();

      imu::for_each([&](const sym_t::p& sym) {
          type_ctx.close_over(sym);
        }, fields);

      emit_type_impls(type->type(), impls, type_ctx);

      t << instr::push << var;
    }

    void new_(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto name   = as<sym_t>(imu::first(forms));
      auto lookup = resolve(ctx, name);

      if (!lookup || !lookup.is_global()) {
        throw std::runtime_error("Type has to refer to a namespace level var");
      }

      auto type = as<var_t>(*lookup)->deref<type_value_t>();
      auto args = imu::rest(forms);

      if (imu::count(args) != imu::count(type->fields())) {
        throw std::runtime_error("Wrong number of args to new");
      }

      compile_all(args, ctx, t);

      if (type->name() == "String") {
        t << instr::make_native<string_t> << 3;
      }
      else if (type->name() == "Symbol") {
        t << instr::make_native<sym_t> << 4;
      }
      else {
        t << instr::make << type;
      }
    }

    void dot(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto x   = *imu::first(forms);
      auto sym = as<sym_t>(imu::second(forms));

      compile(x, ctx, t);
      t << instr::field << sym;
    }

    void set(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto x   = as<sym_t>(imu::first(forms));
      auto sym = as<sym_t>(imu::second(forms));
      auto val = imu::first(imu::drop(2, forms));

      compile(*val, ctx, t);
      compile(x, ctx, t);

      t << instr::set << sym;
    }
  }
}
