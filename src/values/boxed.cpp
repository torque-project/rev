#include "../values.hpp"

#include <string.h>

using namespace rev;

namespace rev {
  value_t::p Int_Printable_str(value_t::p self) {
    return imu::nu<string_t>(std::to_string(as<int_t>(self)->value));
  }

  template<typename T>
  value_t::p Box_Serializable_binary(value_t::p self) {
    auto s    = as<T>(self);
    auto size = sizeof(s->value);
    auto buf  = new uint8_t[size];
    memcpy(buf, &s->value, size);
    return imu::nu<binary_t>(buf, size);
  }

  value_t::p Int_Equiv_equiv(value_t::p self, value_t::p other) {
    auto i = as_nt<int_t>(other);
    return (i && as<int_t>(self)->value == i->value) ?
      sym_t::true_ : sym_t::false_;
  }

  struct type_t::impl_t Int_printable[] = {
    {(intptr_t) Int_Printable_str, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Int_serializable[] = {
    {(intptr_t) Box_Serializable_binary<int_t>, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Int_equiv[] = {
    {0, (intptr_t) Int_Equiv_equiv, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Int_methods[] = {
    {protocol_t::str,          Int_printable},
    {protocol_t::serializable, Int_serializable},
    {protocol_t::equiv,        Int_equiv}
  };

  static const uint64_t size = sizeof(Int_methods) / sizeof(Int_methods[0]);

  template<>
  type_t value_base_t<int_t>::prototype("Integer.0", Int_methods, size);
}

extern "C" {
  value_t::p torque_lang_Integer_new(int64_t i) {
    return imu::nu<int_t>(i);
  }

  type_t::p torque_lang_Integer = &value_base_t<int_t>::prototype;
}
