#include "core.hpp"
#include "reader.hpp"
#include "compiler.hpp"
#include "adapter.hpp"
#include "instructions.hpp"
#include "util.hpp"
#include "builtins/operators.hpp"
#include "builtins/functions.hpp"
#include "specials/def.hpp"
#include "specials/deftype.hpp"
#include "specials/protocol.hpp"
#include "specials/if.hpp"
#include "specials/do.hpp"
#include "specials/fn.hpp"
#include "specials/let.hpp"
#include "specials/binding.hpp"
#include "specials/loop.hpp"
#include "specials/quote.hpp"
#include "specials/ns.hpp"
#include "specials/apply.hpp"
#include "specials/ffi.hpp"

#include <cassert>
#include <fstream>
#include <set>
#include <vector>

#include <unistd.h>

namespace rev {

  typedef void (*special_t)(const list_t::p&, ctx_t&, thread_t&);
  typedef void (*instr_t)(stack_t&, stack_t&, int64_t*&);

  static const char*   BUILTIN_NS    = "torque.core.builtin";
  static const int64_t MAX_FN_LENGTH = 100000;

  /**
   * This holds the runtiem state of the interpreter
   *
   */
  struct runtime_t {

    map_t     namespaces;
    thread_t  code;
    stack_t   stack;
    stack_t   sp;
    stack_t   fp;
    int64_t*  ip;

    ns_t::p  in_ns;
    var_t::p ns;

    // static symbols
    sym_t::p  _ns_;

    std::vector<std::string> sources;

  } rt;

  value_t::p run(int64_t address, int64_t to);

  int64_t* jump(int64_t off) {
    return rt.code.data() + off;
  }

  int64_t finalize_thread(thread_t& t) {

    auto pos = rt.code.size();

    rt.code.reserve(rt.code.size() + t.size());
    rt.code.insert(rt.code.end(), t.begin(), t.end());

    return pos;
  }

  int64_t compute_fn_length(int64_t address) {
    auto code   = rt.code.data() + address;
    auto instr  = code;
    auto length = 0;

    while ((*instr != reinterpret_cast<int64_t>(instr::return_to)) &&
           (length < MAX_FN_LENGTH)) {
      ++instr;
      ++length;
    }

    if (*(code + length) != reinterpret_cast<int64_t>(instr::return_to)) {
      throw std::runtime_error("Fn exceeded maximum code length");
    }

    return length + 1;
  }

  ns_t::p ns() {
    return rt.in_ns;
  }

  ns_t::p ns(const sym_t::p& sym) {
    return as_nt<ns_t>(imu::get(&rt.namespaces, sym));
  }

  ns_t::p ns(const sym_t::p& sym, const ns_t::p& ns) {
    rt.in_ns = ns;
    rt.namespaces.assoc(sym, ns);
    return ns;
  }

  var_t::p intern(const sym_t::p& sym, var_t::p var) {
    assert(rt.in_ns && "No current namespace is bound");
    rt.in_ns->intern(sym, var);
    return var;
  }

  bool is_special(const sym_t::p& sym) {
    static std::set<const std::string> specials = {
      "def", "do", "if", "let*", "binding*", "loop*", "quote", "ns", "fn*",
      "deftype", "defprotocol", "dispatch*", "recur", "new", "set!",
      ".", "apply*", "so*", "import*"
    };
    return sym && (specials.count(sym->name()) == 1);
  }

  ctx_t::lookup_t resolve_nt(const ctx_t& ctx, const sym_t::p& sym) {
    auto ns = as<ns_t>(rt.ns->deref());
    maybe<value_t::p> resolved;
    ctx_t::scope_t    scope;

    if (sym->has_ns()) {
      auto ns_sym = sym_t::ns(sym);
      auto the_ns = imu::get(&ns->aliases, ns_sym);

      if (!the_ns) {
        the_ns = imu::get(&rt.namespaces, ns_sym);
      }

      if (!the_ns) {
        std::runtime_error(sym->ns() + " does not resolve to a namespace");
      }

      scope    = ctx_t::scope_t::global;
      resolved = imu::get(&(as<ns_t>(the_ns)->interned), sym_t::name(sym));
    }
    else {
      scope      = ctx_t::scope_t::local;
      resolved   = imu::get(ctx.local(), sym);

      if (!resolved) {
        scope    = ctx_t::scope_t::env;
        resolved = imu::get(ctx.env(), sym);
      }
      if (!resolved) {
        scope    = ctx_t::scope_t::global;
        resolved = imu::get(&ns->interned, sym);
      }
      if (!resolved) {
        resolved = imu::get(&ns->mappings, sym);
      }
    }

    return {scope, (resolved ? *resolved : nullptr)};
  }

  ctx_t::lookup_t resolve(const ctx_t& ctx, const sym_t::p& sym) {
    if (sym->has_ns() && sym->ns() == BUILTIN_NS) {
      return {ctx_t::scope_t::global, nullptr};
    }
    if(auto resolved = resolve_nt(ctx, sym)) {
      return resolved;
    }
    if (is_special(sym)) {
      return {ctx_t::scope_t::global, nullptr};
    }
    throw std::runtime_error(sym->name() + " is not bound");
  }

  var_t::p resolve(const sym_t::p& sym) {
    if (auto lookup = resolve_nt(ctx_t(), sym)) {
      return as<var_t>(*lookup);
    }
    return nullptr;
  }

  sym_t::p qualify(const sym_t::p& sym) {
    if (auto lookup = resolve_nt(ctx_t(), sym)) {
      if (!sym->has_ns()) {
        return sym_t::intern(lookup.ns()->name(), sym->name());
      }
    }
    return sym;
  }

  value_t::p nativize(const value_t::p& form) {
    if (protocol_t::satisfies(protocol_t::seq, form)) {
      if (auto lst = as_nt<list_t>(form)) {
        return lst;
      }

      return imu::take_while<list_t::p>(
        [](const value_t::p&) {
          return true;
        }, imu::nu<rt_seq_t>(form));
    }
    return form;
  }

  value_t::p macroexpand1(const list_t::p& form, ctx_t& ctx) {

    static auto is_macro = keyw_t::intern("macro");

    if (auto sym = as_nt<sym_t>(imu::first(form))) {
      if (auto lookup = resolve(ctx, sym)) {
        if (lookup.is_global()) {
          auto mac = as_nt<fn_t>(as<var_t>(*lookup)->deref());
          if (mac && has_meta(*lookup, is_macro)) {
            return call(mac, imu::rest(form));
          }
        }
      }
    }

    return form;
  }

  value_t::p macroexpand(const value_t::p& form, ctx_t& ctx) {

    value_t::p result = form;
    value_t::p prev   = nullptr;

    while(result != prev && is<list_t>(result)) {
      prev   = result;
      result = nativize(macroexpand1(as<list_t>(result), ctx));
    }

    return result;
  }

  namespace specials {

    special_t find(sym_t::p& sym) {
      if (sym) {
        if (sym->name() == "def")         { return def;         }
        if (sym->name() == "deftype")     { return deftype;     }
        if (sym->name() == "defprotocol") { return defprotocol; }
        if (sym->name() == "dispatch*")   { return dispatch;    }
        if (sym->name() == "if")          { return if_;         }
        if (sym->name() == "fn*")         { return fn;          }
        if (sym->name() == "do")          { return do_;         }
        if (sym->name() == "let*")        { return let_;        }
        if (sym->name() == "binding*")    { return binding;     }
        if (sym->name() == "loop*")       { return loop;        }
        if (sym->name() == "recur")       { return recur;       }
        if (sym->name() == "quote")       { return quote;       }
        if (sym->name() == "new")         { return new_;        }
        if (sym->name() == ".")           { return dot;         }
        if (sym->name() == "set!")        { return set;         }
        if (sym->name() == "ns")          { return ns;          }
        if (sym->name() == "apply*")      { return apply;       }
        if (sym->name() == "so*")         { return so;          }
        if (sym->name() == "import*")     { return import;      }
      }
      return nullptr;
    }
  }

  namespace builtins {

    void read(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto str = as<string_t>(instr::stack::pop<value_t::p>(s));
      instr::stack::push(s, rev::read(str->_data));
    }

    void load(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto str = as<string_t>(instr::stack::pop<value_t::p>(s));
      instr::stack::push(s, load_ns(str->_data));
    }

    instr_t find(const sym_t::p& sym) {
      if (sym && sym->ns() == BUILTIN_NS) {
        if (sym->name() == "+")   { return add;     }
        if (sym->name() == "-")   { return sub;     }
        if (sym->name() == "*")   { return mul;     }
        if (sym->name() == "/")   { return div;     }
        if (sym->name() == "==")  { return eq;      }
        if (sym->name() == "<=")  { return lte;     }
        if (sym->name() == ">=")  { return gte;     }
        if (sym->name() == "<")   { return lt;      }
        if (sym->name() == ">")   { return gt;      }
        if (sym->name() == "&")   { return bit_and; }
        if (sym->name() == "|")   { return bit_or;  }
        if (sym->name() == "bsl") { return bsl;     }
        if (sym->name() == "bsr") { return bsr;     }

        if (sym->name() == "throw")       { return throw_;     }
        if (sym->name() == "identical?")  { return identical;  }
        if (sym->name() == "satisfies?")  { return satisfies;  }
        if (sym->name() == "type")        { return type;       }
        if (sym->name() == "type?")       { return builtins::is<type_value_t>; }
        if (sym->name() == "symbol?")     { return builtins::is<sym_t>; }
        if (sym->name() == "integer?")    { return builtins::is<int_t>; }
        if (sym->name() == "var?")        { return builtins::is<var_t>; }
        if (sym->name() == "binary?")     { return builtins::is<binary_t>; }
        if (sym->name() == "array?")      { return builtins::is<array_t>;  }
        if (sym->name() == "binary")      { return binary;            }
        if (sym->name() == "make-binary") { return xmake<binary_t>;   }
        if (sym->name() == "bget")        { return xget<binary_t>;    }
        if (sym->name() == "blength")     { return xlength<binary_t>; }
        if (sym->name() == "aget")        { return xget<array_t>;     }
        if (sym->name() == "aset")        { return aset;              }
        if (sym->name() == "alength")     { return xlength<array_t>;  }
        if (sym->name() == "aclone")      { return aclone;            }
        if (sym->name() == "acopy")       { return acopy;             }
        if (sym->name() == "array")       { return array;             }
        if (sym->name() == "make-array")  { return xmake<array_t>;    }

        if (sym->name() == "print") { return print; }
        if (sym->name() == "read")  { return read;  }
        if (sym->name() == "load")  { return load;  }

        throw std::runtime_error(sym->name() + " is not a builtin function");
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
        auto lookup = sym ? resolve_nt(ctx, sym) : ctx_t::lookup_t();
        if (lookup &&
            lookup.is_global() &&
            is<int_t>(as<var_t>(*lookup)->deref())) {
          if (auto f = as<var_t>(*lookup)->deref<int_t>()) {
            // the callee is a pointer, so we try to call this as
            // as native function
            compile_all(args, ctx, t);
            t << instr::native << f << imu::count(args);
          }
        }
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
  }

  void compile(const value_t::p& form, ctx_t& ctx, thread_t& t) {

    if (protocol_t::satisfies(protocol_t::alist, form)) {
      auto lst = as<list_t>(nativize(form));
      if (!imu::is_empty(lst)) {
        auto expanded = macroexpand(lst, ctx);
        if (auto lst = as_nt<list_t>(expanded)) {
          emit::invoke(lst, ctx, t);
        }
        else {
          compile(expanded, ctx, t);
        }
      }
      else {
        t << instr::push << form;
      }
    }
    else if (auto sym = as_nt<sym_t>(form)) {
      if (sym == sym_t::true_ || sym == sym_t::false_) {
        t << instr::push << sym;
      }
      else {
        auto lookup = resolve(ctx, sym);
        assert(lookup && "symbol resolved to nil instead of var");
        if (lookup.is_local()) {
          t << instr::poke << as<int_t>(*lookup)->value;
        }
        else if (lookup.is_global()) {
          t << instr::deref << *lookup;
        }
        else {
          // add lookup result to the list of closed over vars
          t << instr::enclosed << ctx.close_over(sym);
        }
      }
    }
    else if (auto v = as_nt<vector_t>(form)) {
      compile_all(v, ctx, t);
      t << instr::make_native<vector_t> << imu::count(v);
    }
    else if (auto m = as_nt<map_t>(form)) {
      imu::for_each([&](const map_t::value_type& kv) {
          compile(imu::first(kv), ctx, t);
          compile(imu::second(kv), ctx, t);
      }, m);
      t << instr::make_native<map_t> << (imu::count(m) * 2);
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

  value_t::p run(int64_t address, int64_t to) {
    // get pointer to last instruction, which is a call to code
    // we want to eval
    rt.ip    = rt.code.data() + address;
    auto end = rt.code.data() + to;

    // FIXME: we do one additional check per instruction to see if the
    // next operation is the final one in this run. this is done
    // so we can exit the loop when the last operation is a 'return'
    // (which will modify the instruction pointer). however, this is
    // one extra comparison per instruction. since this is a very tight
    // loop we should eliminate this in the future, if at all possible
    bool bail = false;

    while(rt.ip != end && !bail) {
      auto op = *(rt.ip++);
#ifdef _TRACE
      std::cout
        << "op(" << (rt.ip - rt.code.data() - 1) << " / " << op << "): "
        << std::flush;
#endif
      bail = (rt.ip == end);
      ((instr_t) op)(rt.sp, rt.fp, rt.ip);
    }

    auto ret = instr::stack::pop<value_t::p>(rt.sp);

    return ret;
  }

  void verify_stack_integrity(stack_t sp) {
    assert(rt.sp == sp &&
      "You've hit a severe bug that means that the VM stack wasn't unwound " \
      "properly. This is most likely a problem with the VM itself.");
  }

  value_t::p call(
    int64_t from, int64_t to, uint32_t stack,
    value_t::p args[], uint32_t nargs) {
    // save stack pointer for sanity checks and stack unwinding
    stack_t sp = rt.sp;
    // setup the frame for the runtime call
    instr::stack::push(rt.sp, (int64_t) rt.ip);
    instr::stack::push(rt.sp, (int64_t) rt.fp);
    rt.fp = rt.sp;

    for (auto i=0; i<nargs; ++i) {
      instr::stack::push(rt.sp, args[i]);
    }

    rt.sp += stack;
    auto ret = run(from, to);
    verify_stack_integrity(sp);

    return ret;
  }

  value_t::p call(const fn_t::p& f, const list_t::p& args) {
    // save stack pointer for sanity checks and stack unwinding
    stack_t sp = rt.sp;
    // setup the frame for the runtime call
    instr::stack::push(rt.sp, (int64_t) rt.ip);
    instr::stack::push(rt.sp, (int64_t) rt.fp);
    rt.fp = rt.sp;

    instr::stack::push(rt.sp, f);

    imu::for_each([&](const value_t::p& v) {
      instr::stack::push(rt.sp, v);
    },
    args);

    auto arity = (int64_t) imu::count(args);
    auto code  = rt.code.data() + f->code();
    auto off   = *(code + arity);

    if ((arity > f->max_arity()) && f->is_variadic()) {

      off = *(code + f->variadic_arity() + 1);
      if (off != -1) {
        auto rest = list_t::p();
        while(arity-- > f->variadic_arity()) {
          rest = imu::conj(rest, instr::stack::pop<value_t::p>(rt.sp));
        }
        instr::stack::push(rt.sp, rest);
        arity = f->variadic_arity();
      }
    }

    if (off != -1) {
      rt.sp += fn_t::stack_space(off, arity);
      return run(f->code() + fn_t::offset(off), (rt.ip - rt.code.data()));
      // verify_stack_integrity(sp);
    }
    throw std::runtime_error("Arity mismatch when calling fn: " + f->name());
  }

  value_t::p eval(const value_t::p& form) {
#ifdef _TRACE
    std::cout << "eval" << std::endl;
#endif

    // bind current namespace
    rt.ns->bind(rt.in_ns);

    thread_t thread;
    ctx_t    ctx;
    compile(form, ctx, thread);

    // save stack pointer for sanity checks and stack unwinding
    stack_t sp = rt.sp + ctx.stack_space() + 1;

    rt.fp  = rt.sp;
    rt.sp += ctx.stack_space() + 1;

    auto ret = run(finalize_thread(thread), rt.code.size());
    verify_stack_integrity(sp);
    rt.sp = rt.fp;

    // TODO: delete eval code from vm ?
    return ret;
  }

  value_t::p read(const std::string& s) {
    return rdr::read(s);
  }

  void load_file(const std::string& source) {

    ns_t::p cur = rt.in_ns;

    std::fstream file(source);
    if (!file.good()) {
      throw std::runtime_error("Can't open source file: " + source);
    }

    if (!is<ns_t>(eval(rdr::read(file)))) {
      throw std::runtime_error(
        "Expecting file to start with ns declaration: " + source);
    }

    while (file.good()) {
      if (auto o = rdr::read(file)) {
        eval(o);
      }
    }

    if (cur) {
      rt.in_ns = cur;
    }
  }

  ns_t::p load_ns(const sym_t::p& name) {

    auto file = replace(name->name(), '.', '/') + ".trq";

    for (auto& path : rt.sources) {
      auto source = path + '/' + file;
      if (access(source.c_str(), F_OK) != -1) {
        load_file(source);
        return as<ns_t>(imu::get(&rt.namespaces, name));
      }
    }
    return nullptr;
  }

  ns_t::p load_ns(const std::string& name) {
    return load_ns(sym_t::intern(name));
  }

  bool parse_source_paths(const std::string& s) {

    std::stringstream ss(s);

    while (ss.good()) {
      std::string path;
      std::getline(ss, path, ':');
      if (!path.empty()) {
        rt.sources.push_back(path);
      }
    }
    return !rt.sources.empty();
  }

  void boot(uint64_t stack, const std::string& s) {
    rt.fp = rt.sp = rt.stack = new int64_t[stack];

    ns(sym_t::intern("user"), imu::nu<ns_t>("user"));
    rt.ns = imu::nu<var_t>(); rt.ns->bind(rt.in_ns);

    if (parse_source_paths(s)) {
      // load core name space and make it visible in user name space
      auto core = load_ns("torque.core");
      auto eno  = core->intern(sym_t::intern("*errno*"), nu<var_t>());

      eno->bind(nu<int_t>(0));
 
      rt.in_ns->map(core);
    }
  }
}
