#pragma once

#include "ffi.h"

#include <unordered_map>

#include <dlfcn.h>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

#define SO_PREFIX "lib"
#define SO_EXT    ".so"

#if defined (__linux__)
#include <gnu/lib-names.h>
#endif

#if defined (__APPLE__) && defined (__MACH__)
#undef  SO_EXT
#define SO_EXT ".dylib"
#endif

#endif

namespace rev {

  namespace ffi {

    static const auto NAME   = keyw_t::intern("name");
    static const auto RET    = keyw_t::intern("ret");
    static const auto ARGS   = keyw_t::intern("args");
    static const auto NATIVE = keyw_t::intern("native");
    static const auto VOID   = keyw_t::intern("void");
    static const auto BOOL   = keyw_t::intern("bool");
    static const auto SINT32 = keyw_t::intern("sint32");
    static const auto UINT32 = keyw_t::intern("uint32");
    static const auto SINT64 = keyw_t::intern("sint64");
    static const auto UINT64 = keyw_t::intern("uint64");
    static const auto PTR    = keyw_t::intern("ptr");
    static const auto STRING = keyw_t::intern("string");
    static const auto OUT    = keyw_t::intern("out");

    static std::unordered_map<std::string, ffi_type*> convert_type = {
      {"void",   &ffi_type_void},
      {"bool",   &ffi_type_uint32},
      {"sint32", &ffi_type_sint32},
      {"uint32", &ffi_type_uint32},
      {"sint64", &ffi_type_sint64},
      {"uint64", &ffi_type_uint64},
      {"ptr",    &ffi_type_pointer},
      {"string", &ffi_type_pointer},
      {"native", &ffi_type_pointer}
    };

    template<typename T>
    inline void convert_types(const T& types, ffi_type** out) {
      int i=0; // imu::count(types);
      imu::for_each([&](const keyw_t::p& k) {
          if (!(out[i++] = ffi::convert_type[k->name()])) {
            throw std::runtime_error("Unknown ffi type: " + k->name());
          }
        }, types);
    }

    struct marshalled_t {

      typedef std::unique_ptr<marshalled_t> p;

      void* _arg;

      inline marshalled_t(void* arg)
        : _arg(arg)
      {}

      inline void* arg() const {
        return _arg;
      }
    };

    struct marshalled_str_t : public marshalled_t {
      string_t::p _s;
      const char* _ptr;

      inline marshalled_str_t(const string_t::p& s)
        : marshalled_t(0) {
        _s   = s;
        _ptr = s->data().c_str();
        _arg = &_ptr;
      }
    };

    struct marshalled_out_t : public marshalled_t {
      p _ref;

      inline marshalled_out_t(marshalled_t* ref)
        : marshalled_t((void*) &ref->_arg), _ref(ref)
      {}
    };
extern "C" {
    void delegate(ffi_cif* cif, void* ret, void* args[], void* x);
}
    struct fn_ptr_t : public rev::int_t {

      ffi_cif      _cif;
      ffi_closure* _closure;
      ffi_type*    _ret;
      ffi_type**   _types;
      fn_t::p      _f;
      vector_t::p  _sig;

      inline fn_ptr_t(const fn_t::p& f, const vector_t::p& sig)
        : _f(f), _sig(sig) {

        void* out;

        _closure = (ffi_closure*)
          ffi_closure_alloc(sizeof(ffi_closure), &out);

        auto ret  = as<keyw_t>(imu::nth(sig, 0));
        auto args = as<vector_t>(imu::nth(sig, 1));

        _ret   = convert_type[ret->name()];
        _types = new ffi_type*[imu::count(args)];
        convert_types(args, _types);
	
	if (ffi_prep_cif(&_cif, FFI_DEFAULT_ABI, imu::count(args), _ret, _types) != FFI_OK) {
  	  std::cout << "error while creating cif" << std::endl;
        }
        if (ffi_prep_closure_loc(_closure, &_cif, delegate, this, out) != FFI_OK) {
          std::cout << "error while prepping closure" << std::endl;
        }
 
	value = reinterpret_cast<int64_t>(out);
     }

      ~fn_ptr_t() {
        ffi_closure_free(_closure);
        delete[] _types;
      }
    };

    /**
     * Convert a runtime value to a native value of the type
     * indicated by type
     *
     */
    marshalled_t* marshal(const value_t::p& arg, const value_t::p& type) {
      if (auto k = as_nt<keyw_t>(type)) {
        if (!has_meta(k, OUT)) {
          if (keyw_t::equiv(k, PTR)) {
            if (auto i = as_nt<int_t>(arg)) {
              return new marshalled_t((void*) &(i->value));
            }
            return new marshalled_t((void*) &arg);
          }
          if (keyw_t::equiv(k, STRING)) {
            return new marshalled_str_t(as<string_t>(arg));
          }
          if (keyw_t::equiv(k, BOOL)) {
            static const int zero = 0;
            static const int one  = 1;
            return new marshalled_t((void*) &(arg == sym_t::true_ ? one : zero));
          }
          if (keyw_t::equiv(k, SINT32) ||
              keyw_t::equiv(k, UINT32) ||
              keyw_t::equiv(k, SINT64) ||
              keyw_t::equiv(k, UINT64)) {
            return new marshalled_t(&(as<int_t>(arg)->value));
          }
          // TODO: throw
        }
        else {
          auto t = imu::nu<keyw_t>(k->fqn());
          return new marshalled_out_t(marshal(arg, t));
        }
      }/*
      else if (auto v = as_nt<vector_t>(type)) {
        // fns are serialized as pointers
        return new marshalled_t(arg);
        }*/
      // TODO: throw
      return nullptr;
    }

    /**
     * Convert a native value to a value of the runtime type
     * indicated by type
     *
     */
    value_t::p unmarshal(const void* arg, const value_t::p& type) {
      if (auto k = as_nt<keyw_t>(type)) {
        if (k == PTR ||
            k == SINT32 || k == SINT64 ||
            k == UINT32 || k == UINT64) {
          return imu::nu<int_t>((int64_t) arg);
        }
        if (k == VOID) {
          return nullptr;
        }
        if (k == STRING) {
          return imu::nu<string_t>((const char*) arg);
        }
        if (k == NATIVE) {
          return value_t::p(arg);
        }
      }
      // TODO: throw
      return nullptr;
    }
extern "C" {
    void delegate(ffi_cif* cif, void* ret, void* args[], void* x) {
      auto handle = reinterpret_cast<fn_ptr_t*>(x);

      auto ret_type  = imu::nth(handle->_sig, 0);
      auto arg_types = as<vector_t>(imu::nth(handle->_sig, 1));

      auto values = imu::nu<list_t>();

      auto arity = imu::count(arg_types);
      auto i = arity;

      while (i-- > 0) {
        values = imu::conj(values, unmarshal(*((void**) args[i]), imu::nth(arg_types, i)));
      }

      // FIXME: this will not work for strings and the like,
      // which would have to be allocated on the clojure side
      // this should be enforced here
      marshalled_t::p r(marshal(rev::call(handle->_f, values), ret_type));
      *((void**) ret) = *((void**) r->arg());
    } }
  }

  namespace instr {

    void native(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "native: ";
#endif
      static var_t::p eno = nullptr;

      auto f         = as<int_t>(stack::pop<value_t::p>(s));
      auto ret_type  = as<keyw_t>((value_t::p) *(ip++));
      auto nat_type  = ffi::convert_type[ret_type->name()];
      auto arg_types = as<vector_t>((value_t::p) *(ip++));
      auto ptr       = reinterpret_cast<void (*)()>(f->value);
      auto arity     = imu::count(arg_types);

      void* ret = nullptr;

      ffi_cif   cif;
      ffi_type* types[arity];
      void*     values[arity];

      ffi::convert_types(arg_types, types);

      std::vector<ffi::marshalled_t::p> marshalled(arity);

      auto i = arity;
      while (i-- > 0) {
        marshalled[i].reset(
          ffi::marshal(
            stack::pop<value_t::p>(s),
            imu::nth(arg_types, i)));
        values[i] = marshalled[i]->arg();
      }

      errno = 0;

      ffi_prep_cif(&cif, FFI_DEFAULT_ABI, arity, nat_type, types);
      ffi_call(&cif, ptr, &ret, values);

      if (!eno) { eno = resolve(sym_t::intern("torque.ffi/*errno*")); }
      eno->deref<int_t>()->value = errno;

      stack::push(s, ffi::unmarshal(ret, ret_type));
    }
  }

  namespace specials {

    void so(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      void* handle = 0;
      if (auto name = as_nt<string_t>(imu::first(forms))) {
        std::string sysname;
#if defined(__linux__)
        if (name->data() == "c") {
          sysname = LIBC_SO;
        }
        else {
#endif
          sysname = SO_PREFIX + name->data() + SO_EXT;
#if defined(__linux__)
        }
#endif
        handle = dlopen(sysname.c_str(), RTLD_LAZY | RTLD_LOCAL);
      }
      else {
        handle = RTLD_DEFAULT;
      }

      auto opts = imu::rest(forms);

      t << instr::push << imu::nu<int_t>((int64_t) handle);
    }

    void import(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto so  = resolve(as<sym_t>(imu::first(forms)))->deref<int_t>();
      auto sym = as<sym_t>(imu::second(forms));
      auto sig = imu::drop(2, forms);

      auto loaded = dlsym((void*) so->value, sym->name().c_str());
      if (!loaded) {
        throw std::runtime_error("Failed to import: " + sym->name());
      }

      auto call = imu::nu<int_t>((int64_t) loaded);
      call->set_meta(imu::nu<map_t>(ffi::NAME, sym));

      t << instr::push << call;
    }

    void invoke(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto ptr   = *imu::first(forms);
      auto ret   = as<keyw_t>(imu::second(forms));
      auto types = as<vector_t>(imu::first(imu::drop(2, forms)));
      auto args  = imu::drop(3, forms);

      compile_all(args, ctx, t);
      compile(ptr, ctx, t);
      t << instr::native << ret << types;
    }
  }
  namespace builtins {
    void fnptr(stack_t& s, stack_t& fp, int64_t* &ip) {
      using namespace instr::stack;
      auto args = as<vector_t>(pop<value_t::p>(s));
      auto ret  = as<keyw_t>(pop<value_t::p>(s));
      auto fn   = as<fn_t>(pop<value_t::p>(s));
      push(s, imu::nu<ffi::fn_ptr_t>(fn, vector_t::factory(ret, args)));
    }
  }
}
