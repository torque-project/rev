#include "reader.hpp"
#include "values.hpp"
#include "bultins/operators.hpp"
#include "specials/if.hpp"
#include "specials/fn.hpp"

#include <vector>

/**
 * This holds the runtiem state of the interpreter
 *
 */
struct runtime_t {

  // namespaces
  // code
  // execution stack
  // exception stack
  // current ns

} rt;

using value_t = ty::value_t;

typedef std::vector<void*> thread_t;

typedef void (*special_t)(list_t::p& args, imu::map::p& env);


value_t::p resolve(const sym_t::p& sym, imu::map::p& env) {
  // resolve a symbol to a var
}

value_t::p macroexpand1(const value_t::p& form) {
  // expand form if it is a macro
}

value_t::p macroexpand(const value_t::p& form) {
  // expand form as long as it is a macro
}

namespace specials {

  special_t find(sym_t::p& sym) {
    if (sym) {
      if (core::name(sym) == "if")  { return specials::if_; }
      if (core::name(sym) == "fn*") { return specials::fn;  }
      // TODO: more special forms like let*, etc.
    }
    return nullptr;
  }
}

namespace emit {

  void invoke(list_t::p l, imu::map& env) {

    auto head = imu::first(l);
    auto args = imu::rest(l);
    auto sym  = as<sym_t>(head);

    if (auto s = specials::find(sym, env)) {
      s(args, env);
    }
    else {
      // 1) emit list as regular function call
      // 2) emit list as protocol call
      // 3) emit list as native call
    }
  }
}

void compile(imu::value& form, imu::map& env) {
  if (auto lst = as<list_t>(form)) {
    // TODO: auto expanded = macroexpand(lst)
    emit::invoke(lst, env);
  }
  // TODO: handle other types of forms
  else {
    // emit form as operand into code
  }
}

value_t::p eval(imu::value& form) {
  // compile and run form
}

imu::value read(const std::string& s) {
  return rdr::read(s);
}

void boot() {
}
