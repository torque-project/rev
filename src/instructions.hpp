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

      template<typename T>
      inline void push(stack_t& s, const T& x) {
        s.push_back((int64_t) x);
      }
    }

    void push(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "push" << std::endl;
#endif
      stack::push(s, *(ip++));
    }

    void return_here(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "return_here: " << *ip << std::endl;
#endif
      fp = s.size();
      auto off = *(ip++);
      stack::push(s, (int64_t) (ip + off));
    }

    void return_to(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "return_to" << std::endl;
#endif
      auto ret  = stack::pop<>(s);

      // reset stack to calling function
      s.erase(s.begin() + fp + 1, s.end());
      auto addr = stack::pop<int64_t*>(s);

      // set instruction pointer to return address
      ip = addr;

      stack::push(s, ret);
    }

    void pop(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "pop" << std::endl;
#endif
      s.pop_back();
    }

    void br(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "br" << std::endl;
#endif
      ip += *ip;
    }

    void brcond(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "brcond" << std::endl;
#endif
      auto cond = stack::pop<value_t::p>(s);
      auto eoff = *(ip++);

      ip += is_truthy(cond) ? 0 : eoff;
    }

    void bind(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "bind" << std::endl;
#endif
      auto var = (var_t::p) *(ip++);
      var->bind(stack::pop<value_t::p>(s));
    }

    void deref(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "deref" << std::endl;
#endif
      auto var = (var_t::p) *(ip++);
      stack::push(s, (int64_t) var->deref());
    }

    void enclosed(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "enclosed" << std::endl;
#endif
      auto fn = as<fn_t>((value_t::p) s[fp+1]);
      stack::push(s, fn->_closed_overs[*ip++]);
    }

    void closure(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "closure" << std::endl;
#endif
      auto address  = *ip++;
      auto enclosed = *ip++;
      auto fn       = imu::nu<fn_t>(address);

      for (auto i=0; i<enclosed; ++i) {
        fn->enclose(stack::pop<value_t::p>(s));
      }

      stack::push(s, fn);
    }

    void dispatch(stack_t& s, int64_t &fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "dispatch" << std::endl;
#endif
      auto f     = as<fn_t>((value_t::p)(s[fp+1]));
      auto arity = *(ip++);

      // FIXME: it's quite ugly to obtain the jump address this way,
      // but for now i can't think of anything better. real memory
      // addresses can't be obtained while building the code, since
      // compiling may invalidate existing addresses at any time.
      ip = jump(f->_code);

      auto off = ip + arity;
      if (*off != -1) {
        ip += *off;
      }
      // TODO: emit list as protocol call
      // TODO: emit list as native call
      else {
        throw std::runtime_error("Arity mismatch when calling fn");
      }
    }
  }
}
