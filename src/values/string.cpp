#include "../values.hpp"

#include <unordered_map>

namespace rev {

  value_t::p String_Printable_str(value_t::p self) {
    return self;
  }

  value_t::p String_Serializable_binary(value_t::p self) {
    auto s = as<string_t>(self);
    return imu::nu<binary_t>(s->_data.c_str(), s->_data.size());
  }

  value_t::p String_Counted_count(value_t::p self) {
    return imu::nu<int_t>(as<string_t>(self)->count());
  }

  struct type_t::impl_t String_printable[] = {
    {0, (intptr_t) String_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t String_serializable[] = {
    {0, (intptr_t) String_Serializable_binary, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t String_counted[] = {
    {0, (intptr_t) String_Counted_count, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t String_methods[] = {
    {protocol_t::istring,      nullptr},
    {protocol_t::str,          String_printable},
    {protocol_t::serializable, String_serializable},
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
