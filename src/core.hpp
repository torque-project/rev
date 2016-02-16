#pragma once

#include "values.hpp"

#include <string>

namespace rev {

  void boot();

  value_t::p read(const std::string&);
  value_t::p eval(const value_t::p&);
  void compile(const value_t::p& form, const map_t::p& env);

  template<typename T>
  inline bool is_true(const T& x) {
    return x == sym_t::true_;
  }

  template<typename T>
  inline bool is_false(const T& x) {
    return x == sym_t::false_;
  }

  template<typename T>
  inline bool is_truthy(const T& x) {
    return x && as_nt<sym_t>(x) != sym_t::false_;
  }
}
