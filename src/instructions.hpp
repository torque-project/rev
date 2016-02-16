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
        auto& x = s.top(); s.top();
        return (T&) x;
      }
    }

    // runtime instructions of the vm
    void push(stack_t& s, thread_t::iterator& ip) {
      s.push((value_t::p) *(ip++));
    }

    void br(stack_t& s, thread_t::iterator& ip) {
      ip = jump(priv::pop<uint64_t>(s));
    }

    void brrel(stack_t& s, thread_t::iterator& ip) {
      ip += priv::pop<uint64_t>(s);
    }

    void brcond(stack_t& s, thread_t::iterator& ip) {
      auto cond = priv::pop<value_t::p>(s);
      auto eoff = priv::pop<uint64_t>(s);

      ip += is_truthy(cond) ? 0 : eoff;
    }
  }

  namespace op {

    void push(thread_t& t, const value_t::p& x) {
      t << instr::push;
      t << x;
    }
  }
}
