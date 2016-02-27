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

    struct impl_t {
      intptr_t arities[8];
    };

    struct ext_t {
      uint32_t id;
      impl_t*  impls;
    };

    struct native_handle_t;

    // runtime callable function pointers to the protocol
    // implementations. this can be used to call the protocol
    // implementations directly
    uint64_t _num_ext;
    ext_t*   _methods;

    const std::string _name;  // protocol implementations
    int64_t           _code; // start of the protocol code

    std::vector<native_handle_t*> _handles;

    inline type_t()
    {}

    inline type_t(const std::string& n)
      : _name(n)
    {}

    ~type_t();

    inline std::string name() const {
      return _name;
    }

    inline int64_t code() const {
      return _code;
    }

    intptr_t prepare_closure(uint8_t arity, int64_t off);
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

    inline const std::string& name() const {
      return _name;
    }

    inline bool has_ns() const {
      return !_ns.empty();
    }

    static inline sym_t::p name(const sym_t::p& sym) {
      return sym_t::intern(sym->_name);
    }

    static inline sym_t::p ns(const sym_t::p& sym) {
      return sym_t::intern(sym->_ns);
    }

    static p intern(const std::string& fqn);
    /*
    inline friend bool operator==(const p& l, const p& r) {
      return (l == r) ||
        ((l && r) &&
         (l->_name == r->_name) &&
         (l->_ns == r->_ns));
    }
    */
  };

  struct array_t : public value_base_t<array_t> {

    // array of values
  };

  struct ns_t : public value_base_t<ns_t> {

    typedef typename semantics<ns_t>::p p;

    typedef imu::ty::basic_array_map<sym_t::p, value_t::p> mappings_t;

    std::string _name;

    mappings_t interned;
    mappings_t mappings;
    mappings_t aliases;

    inline ns_t(const std::string& name)
      : _name(name)
    {}

    inline std::string name() const {
      return _name;
    }

    inline void intern(const sym_t::p& sym, const var_t::p& v) {
      interned.assoc(sym, v);
    }

    inline void intern(const ns_t::p& ns) {
      reference(interned, ns);
    }

    inline void map(const ns_t::p& ns) {
      reference(mappings, ns);
    }

    inline void alias(const sym_t::p& sym, const ns_t::p& ns) {
      aliases.assoc(sym, ns);
    }

    void reference(mappings_t& m, const ns_t::p& ns) {
      imu::for_each([&](const typename mappings_t::value_type& kv) {
          m.assoc(imu::first(kv), imu::second(kv));
        }, &ns->interned);
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

  struct protocol_t : public value_base_t<protocol_t> {

    typedef typename semantics<protocol_t>::p p;

    static uint64_t id;

    uint64_t    _id;
    std::string _name;
    list_t::p   _meths;

    inline protocol_t(const std::string& name, const list_t::p& meths)
      : _id(id++), _name(name), _meths(meths)
    {}

    inline list_t::p meths() const {
      return _meths;
    }
  };

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

    inline type_t::p type() {
      return &_type;
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

    inline const value_t::p& field(uint64_t idx) {
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

  inline decltype(auto) conj(const list_t::p& l, const value_t::p& x) {
    return imu::conj(l, x);
  }

  inline decltype(auto) conj(const vector_t::p& v, const value_t::p& x) {
    return imu::conj(v, x);
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

namespace imu {

  template<typename T>
  inline typename T::p value_cast(const rev::value_t*& v) {
    std::cout << "value cast" << std::endl;
    return rev::as<T>(v);
  }
}
