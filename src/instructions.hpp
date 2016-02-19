#pragma once

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
        auto& x = s.back(); s.pop_back();
        return (T) x;
      }
    }

    void push(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "push" << std::endl;
#endif
      s.push_back(*(ip++));
    }

    void return_here(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "return_here" << std::endl;
#endif
      auto off = *(ip++);
      s.push_back((int64_t) (ip + off));
    }

    void return_to(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "return_to" << std::endl;
#endif
      auto ret  = stack::pop<>(s);
      auto addr = stack::pop<int64_t*>(s);

      ip = addr;

      s.push_back(ret);
    }

    void pop(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "pop" << std::endl;
#endif
      s.pop_back();
    }

    void br(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "br" << std::endl;
#endif
      ip += *ip;
    }

    void brcond(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "brcond" << std::endl;
#endif
      auto cond = stack::pop<value_t::p>(s);
      auto eoff = *(ip++);

      ip += is_truthy(cond) ? 0 : eoff;
    }

    void bind(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "bind" << std::endl;
#endif
      auto var = (var_t::p) *(ip++);
      var->bind(stack::pop<value_t::p>(s));
    }

    void deref(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "deref" << std::endl;
#endif
      auto var = (var_t::p) *(ip++);
      s.push_back((int64_t) var->deref());
    }

    void dispatch(stack_t& s, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "dispatch" << std::endl;
#endif
      auto f     = as<fn_t>(stack::pop<value_t::p>(s));
      auto arity = *(ip++);

      if (auto meth = f->arity(arity)) {
        // FIXME: it's quite ugly to obtain the jump address this way,
        // but for now i can't think of anything better. real memory
        // addresses can't be obtained while building the code, since
        // compiling may invalidate existing addresses at any time.
        ip = jump(meth.address);
      }
      // TODO: emit list as protocol call
      // TODO: emit list as native call
      else {
        throw std::runtime_error(
          "Arity mismatch when calling: " +
          f->name());
      }
    }
  }
}
