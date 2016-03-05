#include "../values.hpp"

namespace rev {

  value_t::p Map_Associative_contains_key(value_t::p self, value_t::p n) {
    return as<vector_t>(self)->nth(as<int_t>(n)->value);
  }

  value_t::p Map_Associative_assoc(value_t::p s, value_t::p k, value_t::p v) {
    return imu::assoc(as<map_t>(s), k, v);
  }

  value_t::p Map_WithMeta_with_meta(value_t::p self, value_t::p meta) {
    auto m = imu::nu<map_t>(*as<map_t>(self));
    m->set_meta(meta);
    return m;
  }

  struct type_t::impl_t Map_with_meta[] = {
    {0, 0,
     (intptr_t) Map_WithMeta_with_meta,
     0, 0, 0, 0}
  };

  struct type_t::impl_t Map_associative[] = {
    {0, 0,
     (intptr_t) Map_Associative_contains_key,
     0, 0, 0, 0},
    {0, 0, 0,
     (intptr_t) Map_Associative_assoc,
     0, 0, 0, 0},
  };

  struct type_t::ext_t Map_methods[] = {
    {protocol_t::associative, Map_associative},
    {protocol_t::withmeta,    Map_with_meta}
  };

  static const uint64_t size =
    sizeof(Map_methods) / sizeof(Map_methods[0]);

  template<>
  type_t value_base_t<map_tag_t>::prototype(
    "PersistentArrayMap.0", Map_methods, size);
}
