#include "../values.hpp"

namespace rev {

  value_t::p Set_Printable_str(value_t::p self) {
    auto set = imu::seq(as<set_t>(self));
    std::string s = "#{";
    if (auto fst = imu::first(set)) {
      s += str(*fst);
      imu::for_each([&](const value_t::p& v) {
          auto str = rev::str(v);
          s += " " + str;
        }, imu::rest(set));
    }
    s += "}";
    return imu::nu<string_t>(s);
  }

  value_t::p Set_Lookup_lookup2(value_t::p s, value_t::p k) {
    if (auto v = imu::get(as<set_t>(s), k)) {
      return *v;
    }
    return nullptr;
  }

  value_t::p Set_Lookup_lookup3(value_t::p s, value_t::p k, value_t::p d) {
    return imu::get(as<set_t>(s), k, d);
  }

  value_t::p Set_IFn_invoke2(value_t::p self, value_t::p m) {
    return Set_Lookup_lookup3(self, m, nullptr);
  }

  value_t::p Set_IFn_invoke3(value_t::p self, value_t::p m, value_t::p d) {
    return Set_Lookup_lookup3(self, m, d);
  }

  value_t::p Set_Counted_count(value_t::p self) {
    return imu::nu<int_t>(as<map_t>(self)->count());
  }

  struct type_t::impl_t Set_printable[] = {
    {0, (intptr_t) Set_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Set_lookup[] = {
    {0, 0,
     (intptr_t) Set_Lookup_lookup2,
     (intptr_t) Set_Lookup_lookup3,
     0, 0, 0, 0}
  };

  struct type_t::impl_t Set_ifn[] = {
    {0, 0,
     (intptr_t) Set_IFn_invoke2,
     (intptr_t) Set_IFn_invoke3,
     0, 0, 0, 0}
  };

  struct type_t::impl_t Set_counted[] = {
    {0, (intptr_t) Set_Counted_count, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Set_methods[] = {
    {protocol_t::str,         Set_printable},
    {protocol_t::lookup,      Set_lookup},
    {protocol_t::ifn,         Set_ifn},
    //{protocol_t::withmeta,    Set_with_meta},
    {protocol_t::counted,     Set_counted}
  };

  static const uint64_t size =
    sizeof(Set_methods) / sizeof(Set_methods[0]);

  template<>
  type_t value_base_t<set_tag_t>::prototype(
    "PersistentHashSet.0", Set_methods, size);
}
