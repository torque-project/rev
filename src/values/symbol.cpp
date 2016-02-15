#include "../values.hpp"

#include <unordered_map>

namespace rev {

  template<>
  type_t value_base_t<sym_t>::prototype("Symbol.0");

  sym_t::sym_t(const std::string& fqn)
  {
    size_t i = fqn.find_first_of('/');
    if (i > 0 && i != std::string::npos) {

      std::string q = fqn.substr(0, i);
      std::string n = fqn.substr(i + 1);

      if (n.empty()) {
        throw std::runtime_error(
          "Name part of qualified symbol is empty: " + fqn);
      }

      _name = n;
      _ns   = q;
    }
    else {
      _name = fqn;
    };
  }

  sym_t::p sym_t::intern(const std::string& fqn) {

    static std::unordered_map<std::string, sym_t::p> cache;

    auto& sym = cache[fqn];
    return sym ? sym : (sym = imu::nu<sym_t>(fqn));
  }
}
