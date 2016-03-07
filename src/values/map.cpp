#include "../values.hpp"

namespace rev {

  value_t::p Map_Printable_str(value_t::p self) {
    auto m = as<map_t>(self);
    std::string s = "{";
    if (auto fst = imu::first(m)) {
      auto a = rev::str(imu::first(*fst));
      auto b = rev::str(imu::second(*fst));
      s += a->data() + " " + b->data();
      imu::for_each([&](const map_t::value_type& v) {
          auto a = rev::str(imu::first(v));
          auto b = rev::str(imu::second(v));
          s += " " + a->data() + " " + b->data();
        }, imu::rest(m));
    }
    s += "}";
    return imu::nu<string_t>(s);
  }

  value_t::p Map_Associative_contains_key(value_t::p self, value_t::p n) {
    return as<vector_t>(self)->nth(as<int_t>(n)->value);
  }

  value_t::p Map_Associative_assoc(value_t::p s, value_t::p k, value_t::p v) {
    return imu::assoc(as<map_t>(s), k, v);
  }

  value_t::p Map_Lookup_lookup2(value_t::p s, value_t::p k) {
    if (auto v = imu::get(as<map_t>(s), k)) {
      return *v;
    }
    return nullptr;
  }

  value_t::p Map_Lookup_lookup3(value_t::p s, value_t::p k, value_t::p d) {
    return imu::get(as<map_t>(s), as<map_t>(k), d);
  }

  value_t::p Map_WithMeta_with_meta(value_t::p self, value_t::p meta) {
    auto m = imu::nu<map_t>(*as<map_t>(self));
    m->set_meta(meta);
    return m;
  }

  value_t::p Map_Counted_count(value_t::p self) {
    return imu::nu<int_t>(as<map_t>(self)->count());
  }

  struct type_t::impl_t Map_printable[] = {
    {0, (intptr_t) Map_Printable_str, 0, 0, 0, 0, 0, 0}
  };

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

  struct type_t::impl_t Map_lookup[] = {
    {0, 0,
     (intptr_t) Map_Lookup_lookup2,
     (intptr_t) Map_Lookup_lookup3,
     0, 0, 0, 0}
  };

  struct type_t::impl_t Map_counted[] = {
    {0, (intptr_t) Map_Counted_count, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Map_methods[] = {
    {protocol_t::str,         Map_printable},
    {protocol_t::associative, Map_associative},
    {protocol_t::lookup,      Map_lookup},
    {protocol_t::withmeta,    Map_with_meta},
    {protocol_t::counted,     Map_counted}
  };

  static const uint64_t size =
    sizeof(Map_methods) / sizeof(Map_methods[0]);

  template<>
  type_t value_base_t<map_tag_t>::prototype(
    "PersistentArrayMap.0", Map_methods, size);
}
