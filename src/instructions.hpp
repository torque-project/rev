#pragma once

#include <stack>
#include <vector>

namespace rev {

  typedef std::vector<void*>     thread_t;
  typedef std::stack<value_t::p> stack_t;

  thread_t::iterator jump(uint64_t off);

  template<typename T>
  inline thread_t& operator<<(thread_t& t, const T& x) {
    t.push_back((thread_t::value_type) x);
    return t;
  }

  namespace instr {

    namespace priv {

      template<typename T>
      inline T& pop(stack_t& s) {
        auto& x = s.top(); s.pop();
        return (T&) x;
      }
    }

    // runtime instructions of the vm
    void push(stack_t& s, thread_t::iterator& ip) {
      s.push((value_t::p) *(ip++));
    }

    void pop(stack_t& s, thread_t::iterator& ip) {
      s.pop();
    }

    void br(stack_t& s, thread_t::iterator& ip) {
      ip = jump((uint64_t) *ip);
    }

    void brrel(stack_t& s, thread_t::iterator& ip) {
      ip += (uint64_t) *ip;
    }

    void brcond(stack_t& s, thread_t::iterator& ip) {
      auto cond = priv::pop<value_t::p>(s);
      auto eoff = (uint64_t) *ip;

      ip += is_truthy(cond) ? 1 : eoff;
    }

    void bind(stack_t& s, thread_t::iterator& ip) {
      auto var = (var_t::p) *(ip++);
      var->bind(priv::pop<value_t::p>(s));
    }

    void deref(stack_t& s, thread_t::iterator& ip) {
      auto var = (var_t::p) *(ip++);
      s.push(var->deref());
    }
  }
}
