#include "core.hpp"
#include "reader.hpp"
#include "compiler.hpp"
#include "instructions.hpp"
#include "builtins/operators.hpp"
#include "specials/def.hpp"
#include "specials/if.hpp"
#include "specials/do.hpp"
#include "specials/fn.hpp"
#include "specials/let.hpp"
#include "specials/loop.hpp"

#include <cassert>
#include <vector>

namespace rev {

  typedef void (*special_t)(const list_t::p&, ctx_t&, thread_t&);
  typedef void (*instr_t)(stack_t&, int64_t&, int64_t*&);

  const char* BUILTIN_NS = "torque.core.builtin";

  /**
   * This holds the runtiem state of the interpreter
   *
   */
  struct runtime_t {

    map_t     namespaces;
    thread_t  code;
    stack_t   stack;

    ns_t::p    in_ns;
    dvar_t::p  ns;

    // static symbols
    sym_t::p  _ns_;

  } rt;

  thread_t& main_thread() {
    return rt.code;
  }

  int64_t* jump(int64_t off) {
    return rt.code.data() + off;
  }

  int64_t finalize_thread(thread_t& t) {

    auto pos = rt.code.size();

    rt.code.reserve(rt.code.size() + t.size());
    rt.code.insert(rt.code.end(), t.begin(), t.end());

    return pos;
  }

  void intern(const sym_t::p& sym, const var_t::p& var) {
    if (!rt.in_ns) {
      throw std::runtime_error("No current namespace is bound");
    }
    rt.in_ns->intern(sym, var);
  }

  ctx_t::lookup_t resolve(ctx_t& ctx, const sym_t::p& sym) {

    auto ns = as<ns_t>(rt.ns->deref());
    maybe<value_t::p> resolved;
    ctx_t::scope_t    scope;

    if (sym->has_ns()) {

      auto ns_sym = sym_t::intern(sym->ns());
      auto the_ns = imu::get(&ns->aliases, ns_sym);

      if (!the_ns) {
        the_ns = imu::get(&rt.namespaces, ns_sym);
      }

      if (!the_ns) {
        std::runtime_error(sym->ns() + " does not resolve to a namespace");
      }

      scope    = ctx_t::scope_t::global;
      resolved = imu::get(&(as<ns_t>(the_ns)->interned), sym);
    }
    else {

      scope    = ctx_t::scope_t::local;
      resolved = imu::get(ctx.local(), sym);
      if (!resolved) {
        scope    = ctx_t::scope_t::env;
        resolved = imu::get(ctx.env(), sym);
        if (!resolved) {
          scope    = ctx_t::scope_t::global;
          resolved = imu::get(&ns->interned, sym);
          if (!resolved) {
            resolved = imu::get(&ns->mappings, sym);
          }
        }
      }
    }

    if (!resolved) {
      throw std::runtime_error(sym->name() + " is not bound");
    }

    return {scope, as<var_t>(*resolved)};
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
        if (sym->name() == "def")   { return specials::def;   }
        if (sym->name() == "if")    { return specials::if_;   }
        if (sym->name() == "fn*")   { return specials::fn;    }
        if (sym->name() == "do")    { return specials::do_;   }
        if (sym->name() == "let*")  { return specials::let_;  }
        if (sym->name() == "loop*") { return specials::loop;  }
        if (sym->name() == "recur") { return specials::recur; }
        // TODO: implement all special forms
      }
      return nullptr;
    }
  }

  namespace builtins {

    instr_t find(const sym_t::p& sym) {
      if (sym && sym->ns() == BUILTIN_NS) {
        if (sym->name() == "+")  { return builtins::add; }
        if (sym->name() == "-")  { return builtins::sub; }
        if (sym->name() == "*")  { return builtins::mul; }
        if (sym->name() == "/")  { return builtins::div; }
        if (sym->name() == "==") { return builtins::eq; }
        if (sym->name() == "<=") { return builtins::lte; }
        if (sym->name() == ">=") { return builtins::gte; }
        if (sym->name() == "<")  { return builtins::lt; }
        if (sym->name() == ">")  { return builtins::gt; }
      }
      return nullptr;
    }
  }

  namespace emit {
    /*
    void dispatch(const fn_t::p& f, uint8_t arity) {
      if (auto meth = f->arity(arity)) {
        rt.code
          << instr::return_here
          << instr::br << meth.address
          << instr::pop;
      }
      else {
        throw std::runtime_error("Arity mismatch when calling fn");
      }
    }
    */
    void invoke(const list_t::p& l, ctx_t& ctx, thread_t& t) {

      auto head = *imu::first(l);
      auto args = imu::rest(l);
      auto sym  = as_nt<sym_t>(head);

      if (auto s = specials::find(sym)) {
        s(args, ctx, t);
      }
      else if (auto b = builtins::find(sym)) {
        compile_all(args, ctx, t);
        t << b;
      }/* TODO: could implement a direct jump for global fns. this would
          have the benefit of compile time arity errors
      else if (auto f = resolve(ctx.env(), sym)) {
        if (auto fn = as_nt<fn_t>(f->deref())) {
          compile_all(args, ctx, t);
          dispatch(fn, imu::count(args));
        }
        // 2) emit list as protocol call
        // 3) emit list as native call
      }*/
      else {

        // FIXME: this isn't really pretty. maybe it's better to
        // store the return address at the top of the stack, which
        // wouldn't require this hack to patch up the return address
        // after emitting all the arguments. on the other hand binding
        // the parameter in the function becomes more complicated
        // if the return address rests on top of the stack
        t << instr::return_here << 0;
        auto return_addr = t.size();

        compile(head, ctx, t);
        compile_all(args, ctx, t);

        t << instr::dispatch << imu::count(args);
        t[return_addr-1] = (t.size() - return_addr);
      }
    }
  }

  void compile(const value_t::p& form, ctx_t& ctx, thread_t& t) {

    if (auto lst = as_nt<list_t>(form)) {
      // TODO: auto expanded = macroexpand(lst)
      emit::invoke(lst, ctx, t);
    }
    else if (auto sym = as_nt<sym_t>(form)) {
      auto lookup = resolve(ctx, sym);
      assert(lookup && "symbol resolved to nil instead of var");
      if (lookup.is_local() || lookup.is_global()) {
        // this is a local variable of global from a name space
        // in both cases we just put the contents of the var onto
        // the stack
        t << instr::deref << *lookup;
      }
      else {
        // add lookup result to the list of closed over vars
        t << instr::enclosed << ctx.close_over(sym);
      }
    }
    // TODO: handle other types of forms
    else {
      // push literals onto the stack
      t << instr::push << form;
    }
  }

  void compile(const value_t::p& form, ctx_t& ctx) {
    compile(form, ctx, rt.code);
  }

  uint32_t compile_all(const list_t::p& forms, ctx_t& ctx) {
    return compile_all(forms, ctx, rt.code);
  }

  value_t::p eval(const value_t::p& form) {
#ifdef _TRACE
    std::cout << "eval" << std::endl;
#endif
    thread_t thread;
    ctx_t    ctx;
    compile(form, ctx, thread);

    auto address = finalize_thread(thread);

    // get pointer to last instruction, which is a call to code
    // we want to eval
    int64_t fp = 0;
    auto ip    = rt.code.data() + address;
    auto end   = rt.code.data() + rt.code.size();

    while(ip != end) {
      auto op = *(ip++);
#ifdef _TRACE
      std::cout
        << "op(" << (ip - rt.code.data() - 1) << " / " << op << "): "
        << std::flush;
#endif
      ((instr_t) op)(rt.stack, fp, ip);
    }

    auto ret = instr::stack::pop<value_t::p>(rt.stack);
    assert(rt.stack.empty());

    // TODO: delete eval code from vm ?

    return ret;
  }

  value_t::p read(const std::string& s) {
    return rdr::read(s);
  }

  void boot() {
    rt.in_ns = imu::nu<ns_t>();
    rt.ns    = imu::nu<dvar_t>(); rt.ns->bind(rt.in_ns);
  }
}
