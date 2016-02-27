#pragma once

namespace rev {

  namespace specials {

    namespace priv {
      inline ns_t::p get_ns(const sym_t::p& sym) {
        if (auto n = rev::ns(sym)) {
          return n;
        }
        return load_ns(sym);
      }
    }

    void ns(const list_t::p& forms, ctx_t& ctx, thread_t& t) {

      static const auto INHERIT = sym_t::intern("inherit");
      static const auto USE     = sym_t::intern("use");
      static const auto REQUIRE = sym_t::intern("require");
      static const auto AS      = sym_t::intern("as");

      auto name = as<sym_t>(imu::first(forms));
      auto spec = imu::rest(forms);

      auto ns = rev::ns(name, imu::nu<ns_t>(name->name()));

      if (auto core = rev::ns(sym_t::intern("torque.core"))) {
        ns->map(core);
      }

      imu::for_each([&](const list_t::p& references) {
          auto key = as<sym_t>(imu::first(references));
          imu::for_each([&](const vector_t::p& v) {
              auto reference  = seq(v);
              auto sym        = as<sym_t>(imu::first(reference));
              auto referenced = priv::get_ns(sym);
              if (key == INHERIT) {
                ns->intern(referenced);
              }
              else if (key == USE) {
                ns->map(referenced);
              }
              else if (key == REQUIRE) {
                auto opts = imu::partition<list_t::p>(2, imu::rest(reference));
                imu::for_each([&](const list_t::p& x) {
                    auto opt = as<sym_t>(imu::first(x));
                    auto val = as<sym_t>(imu::second(x));
                    if (opt == AS) {
                      ns->alias(val, referenced);
                    }
                  }, opts);
              }
            }, imu::rest(references));
        }, spec);

      t << instr::push << ns;
    }
  }
}
