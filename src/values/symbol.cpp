#include "../values.hpp"

#include <unordered_map>

namespace rev {

  template<>
  type_t value_base_t<sym_t>::prototype("Symbol.0");

  template<>
  type_t value_base_t<keyw_t>::prototype("Keyword.0");

  template<>
  sym_base_t<sym_t>::p sym_base_t<sym_t>::intern(const std::string& fqn) {

    static std::unordered_map<std::string, sym_t::p> cache;

    auto& sym = cache[fqn];
    return sym ? sym : (sym = imu::nu<sym_t>(fqn));
  }

  template<>
  sym_base_t<keyw_t>::p sym_base_t<keyw_t>::intern(const std::string& fqn) {

    static std::unordered_map<std::string, keyw_t::p> cache;

    auto& k = cache[fqn];
    return k ? k : (k = imu::nu<keyw_t>(fqn));
  }

  sym_t::p sym_t::true_  = sym_t::intern("true");
  sym_t::p sym_t::false_ = sym_t::intern("false");
}
