#include "../values.hpp"

namespace rev {

  value_t::p Vector_Printable_str(value_t::p self) {
    auto v = as<vector_t>(self);
    std::string s = "[";
    if (auto fst = imu::first(v)) {
      s += str(*fst)->data();
      imu::for_each([&](const value_t::p& v) {
          auto str = rev::str(v);
          s += " " + str->data();
        }, imu::rest(v));
    }
    s += "]";
    return imu::nu<string_t>(s);
  }

  value_t::p Vector_IIndexed_nth2(value_t::p self, value_t::p n) {
    return as<vector_t>(self)->nth(as<int_t>(n)->value);
  }

  value_t::p Vector_IIndexed_nth3(value_t::p self, value_t::p n, value_t::p d) {
    auto i = as<int_t>(n)->value;
    auto v = as<vector_t>(self);
    if (i >= 0 && i < imu::count(v)) {
      return v->nth(i);
    }
    return d;
  }

  struct type_t::impl_t Vector_printable[] = {
    {0, (intptr_t) Vector_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_iindexed[] = {
    {0, 0,
     (intptr_t) Vector_IIndexed_nth2,
     (intptr_t )Vector_IIndexed_nth3,
     0, 0, 0, 0}
  };

  struct type_t::ext_t Vector_methods[] = {
    {protocol_t::str,     Vector_printable},
    {protocol_t::indexed, Vector_iindexed},
  };

  static const uint64_t size =
    sizeof(Vector_methods) / sizeof(Vector_methods[0]);

  template<>
  type_t value_base_t<vector_tag_t>::prototype(
    "PersistentVector.0", Vector_methods, size);
}
