#include "../values.hpp"

namespace rev {

  value_t::p Var_Deref_deref(value_t::p self) {
    return as<var_t>(self)->deref();
  }

  value_t::p Var_Meta_meta(value_t::p self) {
    auto x = as<var_t>(self);
    return x->meta;
  }

  value_t::p Var_WithMeta_withmeta(value_t::p self, value_t::p m) {
    auto x   = as<var_t>(self);
    auto out = imu::nu<var_t>();

    out->_ns    = x->_ns;
    out->_stack = x->_stack;
    out->set_meta(m);

    return out;
  }

  struct type_t::impl_t Var_deref[] = {
    {(intptr_t) Var_Deref_deref, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Var_withmeta[] = {
    {0, (intptr_t) Var_WithMeta_withmeta, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Var_meta[] = {
    {(intptr_t) Var_Meta_meta, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Var_methods[] = {
    {protocol_t::deref,    Var_deref},
    {protocol_t::meta,     Var_meta},
    {protocol_t::withmeta, Var_withmeta}
  };

  static const uint64_t size = sizeof(Var_methods) / sizeof(Var_methods[0]);

  template<>
  type_t value_base_t<var_t>::prototype("Var.0", Var_methods, size);
}
