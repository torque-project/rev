#pragma once

#include <sstream>

namespace rev {

  namespace builtins {

    using namespace instr::stack;

    void identical(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto a = pop(s);
      auto b = pop(s);
      push(s, a == b ? sym_t::true_ : sym_t::false_);
    }

    void satisfies(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x     = pop<value_t::p>(s);
      auto proto = as<protocol_t>(pop<value_t::p>(s));
      auto ret   = (x && proto->satisfied_by(x->type)) ?
        sym_t::true_ : sym_t::false_;
      push(s, ret);
    }

    void type(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = pop<rt_value_t::p>(s);
      push(s, x ? x->type() : type_value_t::p());
    }

    template<typename T>
    void is(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = pop<value_t::p>(s);
      push(s, is<T>(x) ? sym_t::true_ : sym_t::false_);
    }

    void binary(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto vals = as<list_t>(pop<value_t::p>(s));
      push(s, imu::nu<binary_t>(vals));
    }

    template<typename T>
    void xget(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto n = as<int_t>(pop<value_t::p>(s));
      auto x = as<T>(pop<value_t::p>(s));

      if (n->value >= x->size()) {
        std::stringstream ss;
        ss << "xget: Index out of bounds: "
           << n->value << " >= " << x->size();
        throw std::runtime_error(ss.str());
      }
      push(s, x->get(n->value));
    }

    void aset(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto v = pop<value_t::p>(s);
      auto n = as<int_t>(pop<value_t::p>(s));
      auto a = as<array_t>(pop<value_t::p>(s));

      if (n->value >= a->size()) {
        throw std::runtime_error("Array index out of bounds");
      }
      push(s, a->set(n->value, v));
    }

    void array(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto vals = as<list_t>(pop<value_t::p>(s));
      push(s, imu::nu<array_t>(vals));
    }

    template<typename T>
    void xmake(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto size = as<int_t>(pop<value_t::p>(s));
      push(s, imu::nu<T>(size->value));
    }

    template<typename T>
    void xlength(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = as<T>(pop<value_t::p>(s));
      push(s, imu::nu<int_t>(x->size()));
    }

    void aclone(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto a = as<array_t>(pop<value_t::p>(s));
      push(s, imu::nu<array_t>(a));
    }

    void acopy(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto len = as<int_t>(pop<value_t::p>(s));
      auto off = as<int_t>(pop<value_t::p>(s));
      auto src = as<array_t>(pop<value_t::p>(s));
      auto dst = as<array_t>(pop<value_t::p>(s));

      if (off->value + len->value > src->size()) {
        std::stringstream ss;
        ss
          << "out of bounds while copying array (offset="
          << off->value << ",len=" << len->value << ")";
        throw std::runtime_error(ss.str());
      }

      std::copy(
        src->_arr.begin() + off->value,
        src->_arr.begin() + off->value + len->value,
        dst->_arr.begin());

      push(s, imu::nu<array_t>(dst));
    }

    void print(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto v = pop<value_t::p>(s);
      if (!v) {
        std::cout << "nil" << std::endl;
      }
      else {
        pr(v);
      }
      push(s, nullptr);
    }

    void resolve(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto sym = as<sym_t>(pop<value_t::p>(s));
      auto ns  = as<ns_t>(pop<value_t::p>(s));
      push(s, resolve(ns, sym));
    }

    void the_ns(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = pop<value_t::p>(s);
      if (auto ns  = as_nt<ns_t>(x)) { push(s, ns); }
      if (auto sym = as<sym_t>(x))   { push(s,ns(sym)); };
    }
  }
}
