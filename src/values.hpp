#pragma once

#include "momentum/core.hpp"
#include "momentum/list.hpp"
#include "momentum/vector.hpp"
#include "momentum/array_map.hpp"
#include "momentum/hash_set.hpp"

#include <cassert>
#include <memory>
#include <sstream>

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
    uint32_t _num_ext;
    ext_t*   _methods;

    const std::string _name; // protocol implementations
    int64_t           _code; // start of the protocol code

    std::vector<native_handle_t*> _handles;

    inline type_t()
    {}

    inline type_t(const std::string& n)
      : _name(n)
    {}

    inline type_t(const std::string& n, ext_t* methods, uint64_t num)
      : _name(n), _methods(methods), _num_ext(num)
    {}

    ~type_t();

    inline std::string name() const {
      return _name;
    }

    inline int64_t code() const {
      return _code;
    }

    template<typename T>
    inline std::string sig(const T* p) const {
      std::stringstream ss;
      ss << "#{" << _name << ": " << p << "}";
      return ss.str();
    }

    intptr_t prepare_closure(uint8_t arity, int64_t off);
    void     finalize(int64_t address);
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

    typedef typename semantics<value_t>::p  p;
    typedef typename semantics<value_t>::cp cp;

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

    inline std::string str() const;

    /**
     * Calling these will do a lookup in the values protocol
     * table and run the appropriate method. This may execute
     * interpreted code, if this is a runtime value
     *
     */
    // inline value_t::p first() const;
    // inline value_t::p rest()  const;
    /*
    inline bool is_empty() const {
      return false;
    }
    */
  };

  template<typename T>
  struct value_base_t : public value_t {

    static type_t prototype;

    inline value_base_t()
      : value_t(&prototype)
    {}
  };

  template<typename T>
  type_t value_base_t<T>::prototype;

  struct var_t : public value_base_t<var_t> {

    typedef typename semantics<var_t>::p p;

    std::vector<value_t::p> _stack;
    value_t::p _ns;

    inline var_t()
      : _stack(1)
    {}

    inline value_t::p ns() const {
      return _ns;
    }

    inline void ns(const value_t::p& ns) {
      _ns = ns;
    }

    inline void bind(const value_t::p& v) {
      _stack[0] = v;
    }

    inline value_t::p deref() const {
      return _stack.back();
    }

    template<typename T>
    inline typename T::p deref() const;

    inline void push(const value_t::p& v) {
      _stack.push_back(v);
    }

    inline void pop() {
      assert((_stack.size()) > 1 && "Push without pop");
      _stack.pop_back();
    }

    template<typename F>
    inline value_t::p with_binding(const value_t::p& v, const F& f) {
      push(v); auto r = f(); pop(); return r;
    }
  };

  struct fn_t : public value_base_t<fn_t> {

    typedef typename semantics<fn_t>::p p;
    typedef typename std::vector<value_t::p> values_t;

    void*      _native[8];
    int8_t     _max_arity;
    int8_t     _variadic;
    value_t::p _name;
    int64_t    _code;
    values_t   _closed_overs;

    fn_t(int64_t code, uint64_t arities, const value_t::p& name)
      : _code(code), _name(name)
    {
      _max_arity = (int8_t) (arities & 0xff);
      _variadic  = (int8_t) (arities >> 8);
    }

    inline void enclose(const value_t::p& v) {
      _closed_overs.push_back(v);
    }

    inline int64_t code() const {
      return _code;
    }

    inline int8_t max_arity() const {
      return _max_arity;
    }

    inline int8_t variadic_arity() const {
      return _variadic;
    }

    inline bool is_variadic() const {
      return _variadic != -1;
    }

    inline std::string name() const;

    static inline uint64_t encode(uint64_t off, uint64_t locals) {
      return (off & 0x00000000ffffffff) | (locals << 32);
    }

    static inline uint64_t offset(uint64_t x) {
      return (x & 0x00000000ffffffff);
    }

    static inline uint64_t stack_space(uint64_t x) {
      return (x >> 32);
    }

    static inline uint64_t stack_space(uint64_t x, uint32_t arity) {
      return ((x >> 32) - arity);
    }
  };

  template<typename T>
  struct box_t : public value_base_t<box_t<T>> {

    typedef typename value_t::semantics<box_t>::p p;

    T value; // the boxed value type

    inline box_t() : value() {}

    inline box_t(const T& v)
      : value(v)
    {}
  };

  using int_t = box_t<int64_t>;

  struct binary_t : public value_base_t<binary_t> {

    typedef typename semantics<binary_t>::p p;

    uint64_t       _size;
    const uint8_t* _data;

    inline binary_t() {
    }

    inline binary_t(uint64_t size) {
      _size = size;
      _data = new uint8_t[size];
    }

    inline binary_t(const uint8_t* data, uint64_t size) {
      _size = size;
      _data = data;
    }

    binary_t(const value_t::p& bins);

    ~binary_t() {
      delete[] _data;
    }

    inline uint64_t size() const {
      return _size;
    }

    inline const uint8_t* data() const {
      return _data;
    }

    inline value_t::p get(int64_t n) const {
      return imu::nu<int_t>(_data[n]);
    }
  };

  struct string_t : public value_base_t<string_t> {

    typedef typename semantics<string_t>::p p;

    std::string _data;
    int64_t     _width;

    string_t(const std::string& s);

    inline const std::string& data() const {
      return _data;
    }

    inline const std::string& name() const {
      return _data;
    }

    inline const int64_t count() const {
      return _data.size();
    }

    static p intern(const std::string& s);

    template<typename T>
    static p from_std(const T& b, const T& e) {
      std::string s;
      auto bin = static_cast<binary_t::p>(*b);
      s.append((const char*) bin->data(), bin->size());

      return imu::nu<string_t>(s);
    }
  };

  template<typename T>
  struct sym_base_t : public value_base_t<T> {

    typedef typename value_t::semantics<T>::p p;

    std::string _name;
    std::string _ns;

    sym_base_t(const std::string& fqn) {
      size_t i = fqn.find_first_of('/');
      if (i > 0 && i != std::string::npos) {

        std::string q = fqn.substr(0, i);
        std::string n = fqn.substr(i + 1);

        if (n.empty()) {
          throw std::runtime_error(
            "Name part of qualified symbol is empty: " +
            fqn);
        }

        _name = n;
        _ns   = q;
      }
      else {
        _name = fqn;
      };
    }

    inline const std::string& ns() const {
      return _ns;
    }

    inline const std::string& name() const {
      return _name;
    }

    inline std::string fqn() const {
      return has_ns() ? ns() + "/" + name() : name();
    }

    inline bool has_ns() const {
      return !_ns.empty();
    }

    static inline p name(const p& sym) {
      return intern(sym->_name);
    }

    static inline p ns(const p& sym) {
      return intern(sym->_ns);
    }

    /**
     * Before calling this you have to be sure that at least
     * the first parameter actually derives from sym_base_t
     *
     */
    static bool equiv(const value_t::p& a, const value_t::p& b);

    static p intern(const std::string& fqn);
    static p intern(const std::string& ns, const std::string& name);
  };

  struct sym_t : public sym_base_t<sym_t> {

    typedef typename semantics<sym_t>::p  p;
    typedef typename semantics<sym_t>::cp cp;

    static sym_t::p true_;
    static sym_t::p false_;

    inline sym_t(const std::string& fqn)
      : sym_base_t<sym_t>(fqn)
    {}

    template<typename T>
    static p from_std(const T& b, const T& e) {
      auto fqn = (string_t::p) *(b + 3);
      return sym_t::intern(fqn->data());
    }
  };

  struct keyw_t : public sym_base_t<keyw_t> {

    typedef typename semantics<keyw_t>::p p;

    inline keyw_t(const std::string& fqn)
      : sym_base_t<keyw_t>(fqn)
    {}

    template<typename T>
    static p from_std(const T& b, const T& e) {
      auto fqn = (string_t::p) *(b + 3);
      return keyw_t::intern(fqn->data());
    }
  };

  struct equal_to {
    inline bool operator()(const value_t::p& a, const value_t::p& b) const;
  };

  struct ns_t : public value_base_t<ns_t> {

    typedef typename semantics<ns_t>::p p;

    typedef imu::ty::basic_array_map<sym_t::p, value_t::p, equal_to> mappings_t;
    typedef typename ns_t::mappings_t::value_type mapping_t;

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

    inline value_t::p resolve(const sym_t::p& sym) {
      return imu::get(&interned, sym, nullptr);
    }

    inline void intern(const sym_t::p& sym, var_t::p v) {
      v->ns(this);
      interned.assoc(sym, v);
    }

    inline void intern(const ns_t::p& ns) {
      reference(interned, ns);
    }

    inline void map(const sym_t::p& sym, var_t::p v) {
      mappings.assoc(sym, v);
    }

    inline void map(const ns_t::p& ns) {
      reference(mappings, ns);
    }

    inline void alias(const sym_t::p& sym, const ns_t::p& ns) {
      aliases.assoc(sym, ns);
    }

    void reference(mappings_t& m, const ns_t::p& ns);

    static bool is_public(const mapping_t& kv);
  };

  struct list_tag_t {};
  struct vector_tag_t {};
  struct map_tag_t {};
  struct set_tag_t {};

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
    , rev::equal_to
    , value_base_t<map_tag_t>
    >;

  using set_t = imu::ty::basic_hash_set<
      value_t::p
    , rev::equal_to
    , value_base_t<set_tag_t>
    >;

  /**
   * This is used as a bridge type between the VM and
   * and the runtime code, to provide a seq interface
   * for all of the native collection types
   *
   */
  template<typename T>
  struct seq_adapter_t : public value_base_t<seq_adapter_t<T>> {

    typedef typename T::p value_type_t;
    typedef decltype(imu::seq(value_type_t())) seq_type_t;

    seq_type_t _seq;

    inline seq_adapter_t(const value_type_t& coll)
      : _seq(imu::seq(coll))
    {}

    inline seq_adapter_t(const seq_type_t& s)
      : _seq(s)
    {}

    inline seq_type_t seq() const {
      return _seq;
    }
  };

  struct array_t : public value_base_t<array_t> {

    typedef typename semantics<array_t>::p p;

    std::vector<value_t::p> _arr;

    inline array_t(const array_t::p& cpy)
      : _arr(cpy->_arr)
    {}

    inline array_t(const list_t::p& vs) {
      _arr.reserve(imu::count(vs));
      imu::for_each([&] (const value_t::p& v) {
          _arr.push_back(v);
        }, vs);
    }

    inline array_t(size_t s)
      : _arr(s)
    {}

    inline size_t size() const {
      return _arr.size();
    }

    inline value_t::p get(size_t n) {
      return _arr[n];
    }

    inline array_t::p set(size_t n, const value_t::p& v) {
      _arr[n] = v;
      return this;
    }
  };

  struct protocol_t : public value_base_t<protocol_t> {

    enum id_t {
      str          = 0,
      counted      = 2,
      coll         = 4,
      indexed      = 5,
      seq          = 6,
      next         = 7,
      lookup       = 8,
      associative  = 9,
      imap         = 10,
      mapentry     = 11,
      ivector      = 14,
      deref        = 15,
      meta         = 17,
      withmeta     = 18,
      equiv        = 21,
      seqable      = 23,
      named        = 38,
      alist        = 42,
      ifn          = 43,
      istring      = 44,
      serializable = 45,
      pointer      = 46,
    };

    typedef typename semantics<protocol_t>::p p;

    static uint64_t ids;

    uint64_t    _id;
    std::string _name;
    list_t::p   _meths;

    inline protocol_t(const std::string& name, const list_t::p& meths)
      : _id(ids++), _name(name), _meths(meths)
    {}

    inline uint64_t id() const {
      return _id;
    }

    inline list_t::p meths() const {
      return _meths;
    }

    inline bool satisfied_by(const type_t::cp& type) const {
      return protocol_t::satisfied_by((id_t) _id, type);
    }

    static value_t::p dispatch(uint32_t, uint32_t, const void* args[], uint32_t);

    inline value_t::p dispatch(uint32_t m, const void* args[], uint32_t n) {
      return protocol_t::dispatch(_id, m, args, n);
    }

    template<typename... TS>
    static inline value_t::p dispatch_(id_t id, uint32_t meth, TS... args) {
      const void* ptrs [sizeof...(args)] =
        {reinterpret_cast<const void*>(args)...};
      return protocol_t::dispatch(id, meth, ptrs, sizeof...(args));
    }

    static bool satisfied_by(id_t id, const type_t::cp& type) {
      auto ret = false;
      if (type) {
        for (auto i=0; i<type->_num_ext; ++i) {
          if (type->_methods[i].id == id) {
            ret = true;
            break;
          }
        }
      }
      return ret;
    }

    static inline bool satisfies(id_t id, const value_t::cp v) {
      return (v && satisfied_by(id, v->type));
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

    inline std::string name() const {
      return _type.name();
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

    inline type_value_t::p type() const {
      return _type;
    };

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

  inline decltype(auto) seq(const set_t::p& s) {
    return imu::seq(s);
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

    throw std::runtime_error(
      "Can't cast type " + x->type->name() + " to " +
      T::prototype.name());
  }

  template<typename T, typename S>
  inline T* as(const maybe<S>& x) {
    if (x) {
      return as<T>(*x);
    }
    throw std::runtime_error(
      "Can't cast unset reference to type " +
      T::prototype.name());
  }

  template<typename T, typename S>
  inline T* as_nt(const maybe<S>& x) {
    if (x) {
      return as_nt<T>(*x);
    }
    return nullptr;
  }

  inline bool has_meta(const value_t::p& v, const value_t::p& k) {
    return v->meta && ((bool) imu::get(as<map_t>(v->meta), k));
  }

  inline std::string str(const value_t::p v) {
    if (v) {
      return v->str();
    }
    return "nil";
  }

  inline std::string value_t::str() const {
    if (protocol_t::satisfies(rev::protocol_t::str, this)) {
      auto s = protocol_t::dispatch_(protocol_t::str, 0, this);
      if (protocol_t::satisfies(protocol_t::istring, s)) {
        auto size = as<int_t>(protocol_t::dispatch_(protocol_t::counted, 0, s));
        auto data = as<int_t>(protocol_t::dispatch_(protocol_t::pointer, 0, s));
        std::string out; out.append((const char*) data->value, size->value);
        return out;
      }
    }
    return "<unprintable>";
    //return type->sig(this);
  }

  inline void pr(const value_t::p& v) {
    std::cout << str(v) << std::endl;
  }

  template<typename T>
  inline typename T::p var_t::deref() const {
    return as<T>(_stack.back());
  }

  inline std::string fn_t::name() const {
    return as<sym_t>(_name)->name();
  }

  template<typename T>
  bool sym_base_t<T>::equiv(const value_t::p& a, const value_t::p& b) {
    auto s = as<T>(a);
    auto x = as_nt<T>(b);

    return x && (x->ns() == s->ns() && (x->name() == s->name()));
  }

  inline void ns_t::reference(mappings_t& m, const ns_t::p& ns)  {
    imu::for_each([&](const mapping_t& kv) {
        m.assoc(imu::first(kv), imu::second(kv));
      },
      imu::filter(&ns_t::is_public, &ns->interned));
  }

  /**
   * equality comparison implementation for use in containers
   *
   */
  inline bool equal_to::operator()(
    const value_t::p& a, const value_t::p& b) const {
    if (a == b) {
      return true;
    }
    if (a == nullptr || b == nullptr) {
      return false;
    }
    if ((is<sym_t>(a) || is<keyw_t>(a))) {
      if (!a->meta && !b->meta) {
        // take shortcut if a != b, a is a symbol type, and both don't
        // have meta tags (i.e. are interned)
        return false;
      }
      return is<sym_t>(a) ? sym_t::equiv(a, b) : keyw_t::equiv(a, b);
    }
    const void* args[] = {(void*) a, (void*) b};
    auto res = protocol_t::dispatch(protocol_t::equiv, 0, args, 2);
    return res == sym_t::true_;
  }
}

namespace imu {

  template<typename T>
  inline typename T::p value_cast(const rev::value_t*& v) {
    return rev::as<T>(v);
  }
}
