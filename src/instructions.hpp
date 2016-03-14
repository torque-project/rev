#pragma once

#include <list>

namespace rev {

  int64_t* jump(int64_t off);

  template<typename T>
  inline thread_t& operator<<(thread_t& t, const T& x) {
    t.push_back((thread_t::value_type) x);
    return t;
  }

  namespace instr {

    namespace stack {

      template<typename T = int64_t>
      inline T pop(stack_t& s) {
        return (T) *(--s);
      }

      inline void pop(stack_t& s, int64_t x) {
        s -= x;
      }

      template<typename T = int64_t>
      inline T top(stack_t& s) {
        return (T) *(s - 1);
      }

      template<typename T>
      inline void push(stack_t& s, const T& x) {
        *s++ = reinterpret_cast<int64_t>(x);
      }
    }

    void push(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout
        << "push: " << (int64_t*) *ip << " -> "
        << ((int64_t*) s) << std::endl;
#endif
      auto val = (value_t::p) *ip;
      stack::push(s, *ip++);
    }

    void return_here(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "return_here: " << s << ", " << fp << ", " << *ip << std::endl;
#endif
      auto off = *(ip++);
      stack::push(s, (int64_t) (ip + off));
      stack::push(s, (int64_t) fp);
    }

    void return_to(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "return_to" << std::endl;
#endif
      auto ret  = stack::pop<>(s);
      // reset stack to calling function
      s         = fp;
      fp        = stack::pop<stack_t>(s);
      auto addr = stack::pop<int64_t*>(s);
      // set instruction pointer to return address
      ip = addr;
      stack::push(s, ret);
    }

    void pop(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "pop" << std::endl;
#endif
      stack::pop(s);
    }

    void br(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "br" << std::endl;
#endif
      ip += *ip;
    }

    void brcond(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "brcond" << std::endl;
#endif
      auto cond = stack::pop<value_t::p>(s);
      auto eoff = *(ip++);

      ip += is_truthy(cond) ? 0 : eoff;
    }

    void assign(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "assign: " << fp << ", " << *ip << std::endl;
#endif
      auto off = *ip++;
      auto val = stack::pop<>(s);
      *(fp + off + 1) = val;
    }

    void bind(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "bind: " << *ip << std::endl;
#endif
      auto var = (var_t::p) *ip++;
      auto val = stack::pop<value_t::p>(s);
      var->bind(val);
    }

    void push_bind(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "push_bind: " << *ip << std::endl;
#endif
      imu::for_each([&](const var_t::p& var) {
          var->push(stack::pop<value_t::p>(s));
        }, (list_t::p) *ip++);
    }

    void pop_bind(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "pop_bind: " << *ip << std::endl;
#endif
      imu::for_each([&](const var_t::p& var) {
          var->pop();
        }, (list_t::p) *ip++);
    }

    void make(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "make" << std::endl;
#endif
      auto type = (type_value_t::p) *(ip++);

      std::list<value_t::p> tmp;
      for (auto i=0; i<imu::count(type->fields()); ++i) {
        auto val = stack::pop<value_t::p>(s);
        tmp.push_front(val);
      }

      // TODO: adapt runtime types

      auto val = imu::nu<rt_value_t>(type, vector_t::from_std(tmp));
      stack::push(s, val);
    }

    template<typename T>
    void make_native(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "make" << std::endl;
#endif
      auto n = *(ip++);
      auto b = (value_t**) (s - n);
      auto e = (value_t**) s;
      auto r = T::from_std(b, e);

      stack::pop(s, n);
      stack::push(s, r);
    }

    void set(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "set" << std::endl;
#endif
      auto x   = stack::pop<rt_value_t::p>(s);
      auto val = stack::top<value_t::p>(s);
      auto sym = as<sym_t>((value_t::p) *ip++);

      x->set(sym, val);
    }

    void deref(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "deref: " << *ip << std::endl;
#endif
      auto var = (var_t::p) *(ip++);
      stack::push(s, (int64_t) var->deref());
    }

    void poke(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout
        << "poke: " << "fp: " << (fp + *ip + 1) << ", " << *ip << " -> "
        << ((int64_t*) *(fp + *ip + 1)) << std::endl;
#endif
      auto off = *(ip++);
      stack::push(s, *(fp + off + 1));
    }

    void field(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "field" << std::endl;
#endif
      auto x   = stack::pop<rt_value_t::p>(s);
      auto sym = (sym_t::p) *ip++;
      stack::push(s, x->field(sym));
    }

    void enclosed(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "enclosed" << std::endl;
#endif
      auto v = (value_t::p) *fp;

      if (auto fn = as_nt<fn_t>(v)) {
        stack::push(s, fn->_closed_overs[*ip++]);
      }
      else if (auto val = static_cast<rt_value_t::p>(v)) {
        stack::push(s, val->field(*ip++));
      }
    }

    void closure(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "closure" << std::endl;
#endif
      auto name      = (value_t::p) *ip++;
      auto address   = *ip++;
      auto enclosed  = *ip++;
      auto max_arity = (uint64_t) *ip++;
      auto fn        = imu::nu<fn_t>(address, max_arity, name);

      std::list<value_t::p> tmp;
      for (auto i=0; i<enclosed; ++i) {
        auto val = stack::pop<value_t::p>(s);
        tmp.push_front(val);
      }

      for (auto& i : tmp) { fn->enclose(i); }

      stack::push(s, fn);
    }

    void dispatch(stack_t& s, stack_t& fp, int64_t* &ip) {
#if defined(_TRACE) || defined(_CALLS)
      std::cout << "dispatch: " << std::flush;
#endif
      static sym_t::p invoke = sym_t::intern("torque.lang.protocols/-invoke");
      static fn_t::p  invoke_fn;

      auto arity = *(ip++);

      // set frame pointer to beginning of function in stack
      auto frame = (s - (arity + 1));
      auto val   = (value_t::p) *frame;
      auto f     = as_nt<fn_t>(val);
      /*
      if (!f) {
        if (!invoke_fn) {
          invoke_fn = resolve(invoke)->deref<fn_t>();
        }
        f = invoke_fn;
      }
      */
      if (f) {
#if defined(_TRACE) || defined(_CALLS)
        std::cout << f->name() << "(" << arity << ")" << std::endl;
#endif
        // FIXME: it's quite ugly to obtain the jump address this way,
        // but for now i can't think of anything better. real memory
        // addresses can't be obtained while building the code, since
        // compiling may invalidate existing addresses at any time.
        ip = jump(f->_code);

        auto off = *(ip + arity);

        if (arity > f->max_arity() && f->is_variadic()) {

          off = *(ip + f->variadic_arity() + 1);
          if (off != -1) {

            auto rest = list_t::p();
            while(arity-- > f->variadic_arity()) {
              rest = imu::conj(rest, stack::pop<value_t::p>(s));
            }
            stack::push(s, rest);

            arity = f->variadic_arity();
          }
        }

        if (off != -1) {
          fp  = frame;
          s  += fn_t::stack_space(off, arity);
          ip += fn_t::offset(off);
        }
        else {
          throw std::runtime_error(
            "Arity mismatch when calling fn: " + f->name());
        }
      }
      else if (val) {
#if defined(_TRACE) || defined(_CALLS)
        std::cout
          << val->type->name() << ".-invoke" << "(" << arity << ")"
          << std::endl;
#endif
        auto invoke_arity = arity + 1;
        void* args[invoke_arity];
        for(int i=arity; i>0; --i) {
          args[i] = stack::pop<void*>(s);
        }
        args[0] = val;

        stack::pop(s, 3);
        auto res = protocol_t::dispatch(protocol_t::ifn, 0, args, invoke_arity);
        stack::push(s, res);
      }
      else {
        throw std::runtime_error("Callable is nil");
      }
    }

    void apply(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto args = stack::pop<value_t::p>(s);
      auto f    = as<fn_t>(stack::pop<value_t::p>(s));
      stack::push(s, call(f, as<list_t>(nativize(args))));
    }

    void method(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "method: ";
#endif
      auto proto = as<protocol_t>(stack::pop<value_t::p>(s));
      auto meth  = *(ip++);
      auto arity = *(ip++);
#ifdef _TRACE
      std::cout
        << proto->_name << "[" << meth << "](" << arity << ")"
        << std::endl;
#endif
      void* args[arity];

      // arguments are on the stack in reverse order, so we
      // can pop arguments front to back here
      for (int i=0; i<arity; ++i) {
        args[i] = stack::pop<void*>(s);
      }

      stack::push(s, proto->dispatch(meth, args, arity));
    }
  }
}
