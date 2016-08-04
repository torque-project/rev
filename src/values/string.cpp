#include "../values.hpp"

#include <unordered_map>

#include <string.h>

namespace rev {

  extern value_t::p Binary_Pointer_intptr(value_t::p);

  value_t::p String_Printable_str(value_t::p self) {
    return self;
  }

  value_t::p String_Serializable_binary(value_t::p self) {
    auto s = as<string_t>(self);
    return imu::nu<binary_t>(
      (unsigned char*) s->_data.c_str(),
      s->_data.size());
  }

  value_t::p String_Pointer_intptr(value_t::p self) {
    auto x = as<string_t>(self)->name();
    return Binary_Pointer_intptr(String_Serializable_binary(self));
  }

  value_t::p String_Equiv_equiv(value_t::p self, value_t::p other) {
    auto s = as<string_t>(self);
    auto o = as_nt<string_t>(other);
    return (o && s->data() == o->data()) ? sym_t::true_ : sym_t::false_;
  }

  value_t::p String_Counted_count(value_t::p self) {
    return imu::nu<int_t>(as<string_t>(self)->count());
  }

  struct type_t::impl_t String_printable[] = {
    {(intptr_t) String_Printable_str, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t String_serializable[] = {
    {(intptr_t) String_Serializable_binary, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t String_pointer[] = {
    {(intptr_t) String_Pointer_intptr, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t String_equiv[] = {
    {0, (intptr_t) String_Equiv_equiv, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t String_counted[] = {
    {(intptr_t) String_Counted_count, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t String_methods[] = {
    {protocol_t::istring,      nullptr},
    {protocol_t::str,          String_printable},
    {protocol_t::serializable, String_serializable},
    {protocol_t::pointer,      String_pointer},
    {protocol_t::equiv,        String_equiv},
    {protocol_t::counted,      String_counted}
  };

  static const uint64_t size =
    sizeof(String_methods) / sizeof(String_methods[0]);

  template<>
  type_t value_base_t<string_t>::prototype("String.0", String_methods, size);

  string_t::string_t(const std::string& s)
    : _data(s), _width(1)
  {}

  string_t::p string_t::intern(const std::string& s) {

    static std::unordered_map<std::string, string_t::p> cache;

    auto& str = cache[s];
    return str ? str : (str = imu::nu<string_t>(s));
  }
}
