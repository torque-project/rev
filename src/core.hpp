#pragma once

#include "values.hpp"

#include <string>

namespace rev {

  void boot(uint64_t stack, const std::string& sources);

  value_t::p read(const std::string&);
  value_t::p eval(const value_t::p&);

  value_t::p call(const fn_t::p& f, const list_t::p& args);
  value_t::p call(int64_t addr, int64_t to, uint32_t stack, value_t::p args[], uint32_t n);

  ns_t::p ns();
  ns_t::p ns(const sym_t::p& sym);
  ns_t::p ns(const sym_t::p& sym, const ns_t::p& n);

  ns_t::p load_ns(const std::string& name);
  ns_t::p load_ns(const sym_t::p& name);

  void load_file(const std::string& source);

  void intern(const sym_t::p& sym, const var_t::p& var);

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

  inline bool is_symbol(const value_t::p& x) {
    return is<sym_t>(x);
  }
}
