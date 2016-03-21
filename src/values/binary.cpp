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

  value_t::p Binary_Serializable_binary(value_t::p self) {
    return self;
  }

  value_t::p Binary_Pointer_intptr(value_t::p self) {
    auto data = as<binary_t>(self)->data();
    return imu::nu<int_t>(reinterpret_cast<int64_t>(data));
  }

  struct type_t::impl_t Binary_printable[] = {
    {0, (intptr_t) Binary_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Binary_serializable[] = {
   {0, (intptr_t) Binary_Serializable_binary, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Binary_pointer[] = {
    {0, (intptr_t) Binary_Pointer_intptr, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Binary_methods[] = {
    {protocol_t::str,          Binary_printable},
    {protocol_t::serializable, Binary_serializable},
    {protocol_t::pointer,      Binary_pointer},
  };

  static const uint64_t size =
    sizeof(Binary_methods) / sizeof(Binary_methods[0]);

  template<>
  type_t value_base_t<binary_t>::prototype("Binary.0", Binary_methods, size);

  binary_t::binary_t(const value_t::p& xs) {

    auto bins = as<list_t>(xs);

    _size = imu::reduce([](size_t size, const value_t::p& x) -> size_t {
        if (auto bin = as_nt<binary_t>(x)) {
          return size + bin->size();
        }
        else if (is<int_t>(x)) {
          return size + 1;
        }
        else {
          throw std::runtime_error(
            "Binaries can only be constructed from bytes and" \
            "other binaries");
        }
      }, 0, bins);
    _data = new char[_size];

    imu::reduce([&](size_t pos, const value_t::p& x) -> size_t {
        if (auto bin = as_nt<binary_t>(x)) {
          memcpy(const_cast<char*>(_data) + pos, bin->data(), bin->size());
          return pos + bin->size();
        }
        else if (auto n = as<int_t>(x)) {
          *(const_cast<char*>(_data) + pos) = n->value;
          return pos + 1;
        }
        return pos;
      }, 0, bins);
  }
}
