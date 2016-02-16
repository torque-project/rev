#include "core.hpp"
#include "reader.hpp"
#include "instructions.hpp"
#include "builtins/operators.hpp"
#include "specials/if.hpp"
#include "specials/fn.hpp"

#include <cassert>
#include <vector>

namespace rev {

  typedef void (*special_t)(const list_t::p&, const map_t::p&);
  typedef void (*instr_t)(stack_t&, thread_t::iterator&);

  const char* BUILTIN_NS = "torque.core.builtin";

  /**
   * This holds the runtiem state of the interpreter
   *
   */
  struct runtime_t {

    map_t    namespaces;
    thread_t code;
    stack_t  stack;
    // exception stack

    ns_t::p   in_ns;
    dvar_t::p ns;

    // static symbols
    sym_t::p _ns_;

  } rt;


  thread_t::iterator jump(uint64_t off) {
    return rt.code.begin() + off;
  }

  var_t::p resolve(const map_t::p& env, const sym_t::p& sym) {

    auto ns = as<ns_t>(rt.ns->deref());
    maybe<value_t::p> resolved;

    if (sym->has_ns()) {

      auto ns_sym = sym_t::intern(sym->ns());
      auto the_ns = imu::get(&ns->aliases, ns_sym);

      if (!the_ns) {
        the_ns = imu::get(&rt.namespaces, ns_sym);
      }

      if (!the_ns) {
        std::runtime_error(sym->ns() + " does not resolve to a namespace");
      }

      resolved = imu::get(&(as<ns_t>(the_ns)->interned), sym);
    }
    else {

      resolved = imu::get(env, sym);
      if (!resolved) {
        resolved = imu::get(&ns->interned, sym);
        if (!resolved) {
          resolved = imu::get(&ns->mappings, sym);
        }
      }
    }

    if (!resolved) {
      throw std::runtime_error(sym->name() + " is not bound");
    }

    return as<var_t>(*resolved);
  }

  value_t::p macroexpand1(const value_t::p& form) {
    // expand form if it is a macro
    return form;
  }

  value_t::p macroexpand(const value_t::p& form) {
    // expand form as long as it is a macro
    return form;
  }

  namespace specials {

    special_t find(sym_t::p& sym) {
      if (sym) {
        if (sym->name() == "if")  { return specials::if_; }
        if (sym->name() == "fn*") { return specials::fn;  }
        // TODO: more special forms like let*, etc.
      }
      return nullptr;
    }
  }

  namespace builtins {

    instr_t find(const sym_t::p& sym) {
      if (sym->ns() == BUILTIN_NS) {
        if (sym->name() == "+") { return builtins::add; }
      }
      return nullptr;
    }
  }

  void compile_all(const list_t::p& form, const map_t::p& env);

  namespace emit {

    void invoke(const list_t::p& l, const map_t::p& env) {

      auto head = *imu::first(l);
      auto args = imu::rest(l);
      auto sym  = as<sym_t>(head);

      if (auto s = specials::find(sym)) {
        s(args, env);
      }
      else if (auto b = builtins::find(sym)) {
        compile_all(args, env);
        rt.code.push_back((void*) b);
      }
      else if (auto f = resolve(env, sym)) {
        // 1) emit list as regular function call
        // 2) emit list as protocol call
        // 3) emit list as native call
      }
    }
  }

  void compile(const value_t::p& form, const map_t::p& env) {
    if (auto lst = as_nt<list_t>(form)) {
      // TODO: auto expanded = macroexpand(lst)
      emit::invoke(lst, env);
    }
    // TODO: handle other types of forms
    else {
      // emit form as operand into code
      op::push(rt.code, form);
    }
  }

  void compile_all(const list_t::p& args, const map_t::p& env) {
    auto x = args;
    while (!imu::is_empty(x)) {
      compile(*imu::first(x), env);
      x = imu::rest(x);
    }
  }

  value_t::p eval(const value_t::p& form) {

    uint64_t off = rt.code.size();

    compile(form, imu::nu<map_t>());

    auto ip = jump(off);
    while(ip != rt.code.end()) {
      ((instr_t) *ip)(rt.stack, ++ip);
    }

    auto ret = rt.stack.top(); rt.stack.pop();

    assert(rt.stack.empty());
    return ret;
  }

  value_t::p read(const std::string& s) {
    return rdr::read(s);
  }

  void boot() {
  }
}
