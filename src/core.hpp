#pragma once

#include "values.hpp"

#include <string>

namespace rev {

  void boot();

  value_t::p read(const std::string&);
  value_t::p eval(const value_t::p&);

  template<typename T>
  inline bool is_truthy(const T& x) {
    return x && x != sym_t::false_;
  }
}
