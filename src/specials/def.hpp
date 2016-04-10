#pragma once

namespace rev {

  namespace specials {

    void def(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      auto name = as<sym_t>(imu::first(forms));
      auto init = imu::second(forms);

      if (name->has_ns()) {
        throw std::runtime_error("Def symbol can't have namespace");
      }

      // resolve sym first, since it might have been declared
      auto lookup = resolve_nt(ctx, name);
      auto var    = imu::nu<var_t>();

      if (lookup) {
        auto declared = as<var_t>(*lookup);
        if (declared->ns() != ns()) {
          std::cout
            << "redefining symbol " << name->name()
            << " originally defined in " << as<ns_t>(declared->ns())->name()
            << std::endl;
        }
        else {
          var = declared;
        }
      }

      var->set_meta(name->meta);
      intern(name, var);

      if (init) {
        if (auto doc = as_nt<string_t>(*init)) {
          if (imu::count(forms) > 2) {
            init = imu::first(imu::drop(2, forms));
          }
        }
        compile(*init, ctx, t);
        t << instr::bind << var;
      }
      t << instr::push << var;
    }
  }
}
