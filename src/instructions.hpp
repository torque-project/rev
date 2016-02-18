#pragma once

namespace rev {

  template<typename T>
  inline thread_t& operator<<(thread_t& t, const T& x) {
    t.push_back((thread_t::value_type) x);
    return t;
  }

  namespace instr {

    namespace stack {

      template<typename T>
      inline T pop(stack_t& s) {
        auto& x = s.back(); s.pop_back();
        return (T) x;
      }
    }

    void push(stack_t& s, int64_t* &ip) {
      s.push_back(*(ip++));
    }

    void return_here(stack_t& s, int64_t* &ip) {
      s.push_back((int64_t) ip);
    }

    void return_to(stack_t& s, int64_t* &ip) {
      ip = (int64_t*) *(s.rbegin() + 1);
    }

    void pop(stack_t& s, int64_t* &ip) {
      s.pop_back();
    }

    void br(stack_t& s, int64_t* &ip) {
      ip += *ip;
    }

    void brcond(stack_t& s, int64_t* &ip) {
      auto cond = stack::pop<value_t::p>(s);
      auto eoff = *(ip++);

      ip += is_truthy(cond) ? 0 : eoff;
    }

    void bind(stack_t& s, int64_t* &ip) {
      auto var = (var_t::p) *(ip++);
      var->bind(stack::pop<value_t::p>(s));
    }

    void deref(stack_t& s, int64_t* &ip) {
      auto var = (var_t::p) *(ip++);
      s.push_back((int64_t) var->deref());
    }
  }
}
