#pragma once

#include "momentum/core.hpp"
#include "momentum/list.hpp"
#include "momentum/vector.hpp"
#include "momentum/array_map.hpp"

#include <memory>

namespace rev {

  struct type_t {

    typedef type_t* p;
    typedef type_t const* cp;

    // protocol implementations
    const std::string name;

    inline type_t()
    {}

    inline type_t(const std::string& n)
      : name(n)
    {}
  };

  struct value_t {

    template<typename T>
    struct semantics {

      typedef T* p;
      typedef T const* cp;

      template<typename... TS>
      static inline p allocate(TS... args) {
        // TODO: replace with garbage collector
        return new T(args...);
      }
    };

    typedef typename semantics<value_t>::p p;

    // runtime type information for this value
    type_t::cp type;
    value_t::p meta;

    inline value_t(type_t::cp t)
      : type(t), meta(nullptr)
    {}

    inline void set_meta(const value_t::p& m) {
      meta = m;
    }

    template<typename F>
    inline void alter_meta(const F& f) {
      set_meta(f(meta));
    }
  };

  template<typename T>
  struct value_base_t : public value_t {

    static type_t prototype;

    inline value_base_t()
      : value_t(&prototype)
    {}
  };

  struct var_t : public value_base_t<var_t> {

    typedef typename semantics<var_t>::p p;

    value_t::p _top;

    inline void bind(const value_t::p& v) {
      _top = v;
    }

    inline value_t::p deref() const {
      return _top;
    }

    template<typename T>
    inline typename T::p deref() const;
  };

  struct dvar_t : public var_t {

    typedef typename semantics<dvar_t>::p p;

    // binding stack
  };

  struct fn_t : public value_base_t<fn_t> {

    typedef typename semantics<fn_t>::p p;
    typedef typename std::vector<value_t::p> values_t;

    // this stores function pointer to call the fn directly
    void*    _native[8];
    int64_t  _code;
    values_t _closed_overs;
    uint8_t  _max_arity;
    bool     _is_macro;

    fn_t(int64_t code, uint8_t max_arity)
      : _code(code), _max_arity(max_arity), _is_macro(false)
    {}

    inline void enclose(const value_t::p& v) {
      _closed_overs.push_back(v);
    }

    inline int64_t code() const {
      return _code;
    }

    inline bool is_macro() const {
      return _is_macro;
    }

    inline uint8_t max_arity() const {
      return _max_arity;
    }
  };

  template<typename T>
  struct box_t : public value_base_t<box_t<T>> {

    typedef typename value_t::semantics<box_t>::p p;

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

    typedef typename semantics<sym_t>::p  p;
    typedef typename semantics<sym_t>::cp cp;

    static sym_t::p true_;
    static sym_t::p false_;

    std::string _name;
    std::string _ns;

    sym_t(const std::string& fqn);

    inline const std::string& ns() const {
      return _ns;
    }

    inline bool has_ns() const {
      return !_ns.empty();
    }

    inline const std::string& name() const {
      return _name;
    }

    static p intern(const std::string& fqn);
  };

  struct array_t : public value_base_t<array_t> {

    // array of values
  };

  struct ns_t : public value_base_t<ns_t> {

    typedef typename semantics<ns_t>::p p;

    typedef imu::ty::basic_array_map<sym_t::p, value_t::p> mappings_t;

    mappings_t interned;
    mappings_t mappings;
    mappings_t aliases;

    inline void intern(const sym_t::p& s, const var_t::p& v) {
      interned.assoc(s, v);
    }
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

  /**
   * Wraps a type_t as a runtime type, so we can return it
   * as a value in the VM
   *
   */
  struct type_value_t : public value_base_t<type_value_t> {

    typedef typename value_t::semantics<type_value_t>::p p;

    type_t   _type;
    map_t::p _fields;

    inline type_value_t(const std::string& name, const vector_t::p& fields)
      : _type(name), _fields(imu::nu<map_t>())
    {
      _fields = imu::reduce(
        [](const map_t::p& m, const value_t::p& field) {
          return imu::assoc(m, field, field);
        }, _fields, fields);
    }

    inline map_t::p fields() const {
      return _fields;
    }

    inline decltype(auto) field(const sym_t::p& sym) {
      return _fields->find(sym);
    }

    inline type_t::cp type() const {
      return &_type;
    }
  };

  /**
   * A value of a type defined by user code (i.e. through deftype)
   *
   */
  struct rt_value_t : public value_t {

    typedef typename value_t::semantics<rt_value_t>::p p;

    type_value_t::p _type;
    vector_t::p     _fields;

    inline rt_value_t(const type_value_t::p& t, const vector_t::p& fields)
      : value_t(t->type()),
        _type(t),
        _fields(fields)
    {}

    inline const value_t::p& field(const sym_t::p& sym) {
      auto idx = _type->field(sym);
      if (idx == -1) {
        throw std::runtime_error("Value doesn't have field: " + sym->name());
      }
      return imu::nth(_fields, idx);
    }

    inline void set(const sym_t::p& sym, const value_t::p& v) {
      auto idx = _type->field(sym);
      if (idx == -1) {
        throw std::runtime_error("Value doesn't have field: " + sym->name());
      }
      _fields = imu::assoc(_fields, idx, v);
    }
  };

  // c++ ADL requires that these functions be defined in the same
  // namespace as their type
  inline decltype(auto) seq(const map_t::p& m) {
    return imu::seq(m);
  }

  inline decltype(auto) conj(const map_t::p& m, const map_t::value_type& x) {
    return imu::conj(m, x);
  }
  // end ADL

  template<typename T>
  inline bool is(const value_t::p& x) {
    return x && (x->type == &T::prototype);
  }

  template<typename T>
  inline bool is(const maybe<value_t* const&>& x) {
    return x && *x && ((*x)->type == &T::prototype);
  }

  template<typename T>
  inline T* as_nt(const value_t::p& x) noexcept {

    if (!x) { return nullptr; }

    if (is<T>(x)) {
      return static_cast<T*>(x);
    }

    return nullptr;
  }


  template<typename T>
  inline T* as(const value_t::p& x) {

    if (!x) { return nullptr; }

    if (is<T>(x)) {
      return static_cast<T*>(x);
    }

    throw std::bad_cast();
  }

  template<typename T, typename S>
  inline T* as(const maybe<S>& x) {
    if (x) {
      return as<T>(*x);
    }
    throw std::domain_error(
      "Passed unset maybe instance to 'as'");
  }

  template<typename T, typename S>
  inline T* as_nt(const maybe<S>& x) {
    if (x) {
      return as<T>(*x);
    }
    return nullptr;
  }

  template<typename T>
  inline typename T::p var_t::deref() const {
    return as<T>(_top);
  }
}
