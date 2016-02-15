#pragma once

#include "semantics.hpp"

#include "momentum/core.hpp"
#include "momentum/list.hpp"
#include "momentum/vector.hpp"
#include "momentum/array_map.hpp"

#include <memory>

namespace rev {

  struct type_t {

    typedef type_t* p;

    // protocol implementations
    const char* name;

    inline type_t()
    {}

    inline type_t(const char* n)
      : name(n)
    {}
  };

  struct value_t {

    typedef std::shared_ptr<value_t> p;

    // runtime type information for this value
    const type_t* type;

    inline value_t(const type_t* t)
      : type(t)
    {}
  };

  template<typename T>
  struct value_base_t : public value_t {

    static type_t prototype;

    inline value_base_t()
      : value_t(&prototype)
    {}
  };

  struct var_t : public value_base_t<var_t> {

    // handle to value
  };

  struct dvar_t : public var_t {

    // binding stack
  };

  struct fn_t : public value_base_t<fn_t> {

    void* arities[8];
    // locals
  };

  template<typename T>
  struct box_t : public value_base_t<box_t<T>> {

    T value; // the boxed value type

    inline box_t(const T& v)
      : value(v)
    {}
  };

  using int_t = box_t<int64_t>;

  struct binary_t : public value_base_t<binary_t> {

    uint64_t    _size;
    const char* _data;
  };

  struct string_t : public value_base_t<string_t> {

    std::string _data;
    int64_t     _width;

    string_t(const std::string& s);

    inline std::string data() const {
      return _data;
    }

    inline std::string name() const {
      return _data;
    }

    static p intern(const std::string& s);
  };

  struct sym_t : public value_base_t<sym_t> {

    std::string _name;
    std::string _ns;

    sym_t(const std::string& fqn);

    inline const std::string& ns() const {
      return _ns;
    }

    inline const std::string& name() const {
      return _name;
    }

    static p intern(const std::string& fqn);
  };

  struct array_t : public value_base_t<array_t> {

    // array of values
  };

  struct list_tag_t {};
  struct vector_tag_t {};
  struct map_tag_t {};

  using list_t = imu::ty::basic_list<
      value_t::p
    , value_base_t<list_tag_t>
    >;

  using vector_t = imu::ty::basic_vector<
      value_t::p
    , value_base_t<vector_tag_t>
    >;

  using map_t = imu::ty::basic_array_map<
      value_t::p
    , value_t::p
    , value_base_t<map_tag_t>
    >;

  template<typename T>
  inline std::shared_ptr<T> as(const value_t::p& v) {

    if (!v) { return nullptr; }

    if (v->type == &T::prototype) {
      return std::static_pointer_cast<T>(v);
    }
    throw std::bad_cast();
  }
}
