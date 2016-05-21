#include "../values.hpp"

namespace rev {

  value_t::p Var_Deref_deref(value_t::p self) {
    return as<var_t>(self)->deref();
  }

  struct type_t::impl_t Var_deref[] = {
    {0, (intptr_t) Var_Deref_deref, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Var_methods[] = {
    {protocol_t::deref, Var_deref}
  };

  static const uint64_t size = sizeof(Var_methods) / sizeof(Var_methods[0]);

  template<>
  type_t value_base_t<var_t>::prototype("Var.0", Var_methods, size);
}
