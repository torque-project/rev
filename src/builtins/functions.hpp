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

    void is_type(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = pop<value_t::p>(s);
      push(s, is<type_value_t>(x) ? sym_t::true_ : sym_t::false_);
    }

    void is_integer(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = pop<value_t::p>(s);
      push(s, is<int_t>(x) ? sym_t::true_ : sym_t::false_);
    }

    void is_symbol(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = pop<value_t::p>(s);
      push(s, is<sym_t>(x) ? sym_t::true_ : sym_t::false_);
    }

    void binary(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto vals = as<list_t>(pop<value_t::p>(s));
      push(s, imu::nu<binary_t>(vals));
    }

    void aget(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto n = as<int_t>(pop<value_t::p>(s));
      auto a = as<array_t>(pop<value_t::p>(s));

      if (n->value >= a->size()) {
        throw std::runtime_error("Array index out of bounds");
      }
      push(s, a->get(n->value));
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

    void is_array(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x = pop<value_t::p>(s);
      push(s, is<array_t>(x) ? sym_t::true_ : sym_t::false_);
    }

    void array(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto vals = as<list_t>(pop<value_t::p>(s));
      push(s, imu::nu<array_t>(vals));
    }

    void make_array(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto size = as<int_t>(pop<value_t::p>(s));
      push(s, imu::nu<array_t>(size->value));
    }

    void alength(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto a = as<array_t>(pop<value_t::p>(s));
      push(s, imu::nu<int_t>(a->size()));
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

    void throw_(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto x   = pop<value_t::p>(s);
      auto msg = str(x);
      throw std::runtime_error(msg->data());
    }

    void print(stack_t& s, stack_t& fp, int64_t* &ip) {
      auto v = pop<value_t::p>(s);
      if (!v) {
        std::cout << "nil" << std::endl;
      }
      else {
        std::cout << str(v)->data() << std::endl;
      }
      push(s, nullptr);
    }
  }
}
