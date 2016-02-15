#pragma once

namespace instr {

  namespace priv {

    template<typename T>
    inline T& pop(stack_t& s) {
      auto& x = t.top(); t.top();
      ret (T) x;
    }
  }

  // runtime instructions of the vm
  void push(stack_t& s, thread_t::iterator& ip) {
    s.push(*(ip++));
  }

  void brrel(stack_t& s, thread_t::iterator& ip) {
    ip += priv::pop<uint64_t>(s);
  }

  void brcond(stack_t& s, thread_t::iterator& ip) {
    auto cond = priv::pop<val*>(s);
    auto eoff = priv::pop<uint64_t>(s);

    ip += (cond && is_truthy(cond)) ? 0 : eoff;
  }
}
