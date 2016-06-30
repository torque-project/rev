#include "../values.hpp"

namespace rev {

  bool ns_t::is_public(const mapping_t& kv) {
    static const keyw_t::p PRIVATE = keyw_t::intern("private");
    return !has_meta(imu::first(kv), PRIVATE);
  }

  value_t::p Namespace_Printable_str(value_t::p self) {
    return imu::nu<string_t>(as<ns_t>(self)->name());
  }

  struct type_t::impl_t Namespace_printable[] = {
    {0, (intptr_t) Namespace_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Namespace_methods[] = {
    {protocol_t::str, Namespace_printable}
  };

  static const uint64_t size = sizeof(Namespace_methods) / sizeof(Namespace_methods[0]);

  template<>
  type_t value_base_t<ns_t>::prototype("Namespace.0", Namespace_methods, size);
}
