#include "../values.hpp"

namespace rev {

  value_t::p Binary_Printable_str(value_t::p self) {
    auto b = as<binary_t>(self);
    std::string s;
    s += "#{binary: ";
    s.append(b->data(), b->size());
    s += "}";
      return imu::nu<string_t>(s);
  }

  value_t::p Binary_Pointer_intptr(value_t::p self) {
    auto data = as<binary_t>(self)->data();
    return imu::nu<int_t>(reinterpret_cast<int64_t>(data));
  }

  struct type_t::impl_t Binary_printable[] = {
    {0, (intptr_t) Binary_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Binary_pointer[] = {
    {0, (intptr_t) Binary_Pointer_intptr, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Binary_methods[] = {
    {protocol_t::str,     Binary_printable},
    {protocol_t::pointer, Binary_pointer},
  };

  static const uint64_t size =
    sizeof(Binary_methods) / sizeof(Binary_methods[0]);

  template<>
  type_t value_base_t<binary_t>::prototype("Binary.0", Binary_methods, size);
}
