#pragma once

#include <map>

#include <dlfcn.h>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))

#define SO_PREFIX "lib"
#define SO_EXT    ".so"

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
    static const auto VOID   = keyw_t::intern("void");
    static const auto SINT32 = keyw_t::intern("sint32");
    static const auto SINT64 = keyw_t::intern("sint64");
    static const auto PTR    = keyw_t::intern("ptr");
    static const auto STRING = keyw_t::intern("string");
    static const auto OUT    = keyw_t::intern("out");

    static std::map<keyw_t::p, ffi_type*> convert_type = {
      {VOID,   &ffi_type_void},
      {SINT32, &ffi_type_sint32},
      {SINT64, &ffi_type_sint64},
      {PTR,    &ffi_type_pointer},
      {STRING, &ffi_type_pointer}
    };

    struct marshalled_t {
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
      std::unique_ptr<marshalled_t> _ref;

      inline marshalled_out_t(marshalled_t* ref)
        : marshalled_t((void*) &ref->_arg), _ref(ref)
      {}
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
              return new marshalled_t((void*) &(as<int_t>(arg)->value));
            }
            return new marshalled_t((void*) &arg);
          }
          if (keyw_t::equiv(k, STRING)) {
            return new marshalled_str_t(as<string_t>(arg));
          }
          if (keyw_t::equiv(k, SINT32) || keyw_t::equiv(k, SINT64)) {
            return new marshalled_t(&(as<int_t>(arg)->value));
          }
          // TODO: throw
        }
        else {
          auto t = imu::nu<keyw_t>(k->fqn());
          return new marshalled_out_t(marshal(arg, t));
        }
      }
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
        if (k == PTR || k == SINT64 || k == SINT32) {
          return imu::nu<int_t>((int64_t) arg);
        }
        if (k == VOID) {
          return nullptr;
        }
      }
      // TODO: throw
      return nullptr;
    }
  }

  namespace instr {

    void native(stack_t& s, stack_t& fp, int64_t* &ip) {
#ifdef _TRACE
      std::cout << "native: ";
#endif
      static var_t::p eno = nullptr;

      auto f         = as<int_t>((value_t::p) *(ip++));
      auto meta      = as<map_t>(f->meta);
      auto ret_type  = as<keyw_t>(imu::get(meta, ffi::RET));
      auto nat_type  = ffi::convert_type[ret_type];
      auto arg_types = as<list_t>(imu::get(meta, ffi::ARGS));
      auto arity     = *(ip++);
      auto ptr       = reinterpret_cast<void (*)()>(f->value);
      void* ret      = nullptr;

      ffi_cif   cif;
      ffi_type* types[arity];
      void*     values[arity];

      std::vector<std::unique_ptr<ffi::marshalled_t>> marshalled(arity);

      for (int i=(arity-1); i>=0; --i) {
        types[i] = &ffi_type_pointer;

        marshalled[i].reset(
          ffi::marshal(
            stack::pop<value_t::p>(s),
            *imu::first(arg_types)));
        arg_types = imu::rest(arg_types);
        values[i] = marshalled[i]->arg();
      }

      ffi_prep_cif(&cif, FFI_DEFAULT_ABI, arity, nat_type, types);
      ffi_call(&cif, ptr, &ret, values);

      if (!eno) { eno = resolve(sym_t::intern("*errno*")); }
      eno->deref<int_t>()->value = errno;

      stack::push(s, ffi::unmarshal(ret, ret_type));
    }
  }

  namespace specials {

    void so(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto name    = as<string_t>(imu::first(forms));
      auto sysname = SO_PREFIX + name->data() + SO_EXT;
      auto opts    = imu::rest(forms);

      auto handle = dlopen(sysname.c_str(), RTLD_LAZY | RTLD_LOCAL);
      t << instr::push << imu::nu<int_t>((int64_t) handle);
    }

    void import(const list_t::p& forms, ctx_t& ctx, thread_t& t) {
      auto so     = resolve(as<sym_t>(imu::first(forms)))->deref<int_t>();
      auto sym    = as<sym_t>(imu::second(forms));
      auto sig    = imu::drop(2, forms);
      auto loaded = dlsym((void*) so->value, sym->name().c_str());
      auto call   = imu::nu<int_t>((int64_t) loaded);
      auto ret    = as<keyw_t>(imu::first(sig));
      auto args   = as<list_t>(imu::second(sig));

      call->set_meta(
        imu::nu<map_t>(
          ffi::NAME, sym,
          ffi::RET,  ret,
          ffi::ARGS, imu::into<list_t::p>(imu::nu<list_t>(), args)));

      t << instr::push << call;
    }
  }
}
