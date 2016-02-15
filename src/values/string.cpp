#include "../values.hpp"

#include <unordered_map>

namespace rev {

  template<>
  type_t value_base_t<string_t>::prototype("String.0");

  string_t::string_t(const std::string& s)
    : _data(s), _width(1)
  {}

  string_t::p string_t::intern(const std::string& s) {

    static std::unordered_map<std::string, string_t::p> cache;

    auto& str = cache[s];
    return str ? str : (str = imu::nu<string_t>(s));
  }
}
