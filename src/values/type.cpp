#include "../values.hpp"

#include <cassert>

#include <ffi.h>

namespace rev {

  extern value_t::p invoke(int64_t, value_t::p args[], uint32_t);

  template<>
  type_t value_base_t<type_value_t>::prototype("Type");

  struct type_t::native_handle_t {
    ffi_cif*     cif;
    ffi_closure* closure;
    ffi_type**   args;
    ~native_handle_t() {
      ffi_closure_free(closure);
      delete args;
      delete cif;
    }
  };

  void call(ffi_cif *cif, void* ret, void* ptrs[], void* off) {

    auto pad  = (value_t**) ret;
    auto self = *((value_t**) ptrs[0]);
    auto code = self->type->_code + (int64_t) off;

    value_t::p args[cif->nargs+1];
    args[0] = self;
    for (int i=0; i<cif->nargs; ++i) {
      args[i] = *((value_t**) ptrs[0]);
    }

    *pad = invoke(code, args, cif->nargs);
  }

  type_t::~type_t() {
    for (auto i : _handles) {
      delete i;
    }
  }

  intptr_t type_t::prepare_closure(uint8_t arity, int64_t off) {

    void* ret;

    ffi_cif* cif = new ffi_cif();
    ffi_closure* closure =
      (ffi_closure*) ffi_closure_alloc(sizeof(ffi_closure), &ret);

    auto handle = new native_handle_t{cif, closure};
    handle->args = new ffi_type*[arity];
    for (int i=0; i<arity; ++i) { handle->args[i] = &ffi_type_pointer; }

    auto code = ffi_prep_cif(
      cif, FFI_DEFAULT_ABI, arity, &ffi_type_pointer, handle->args);
    assert(code == FFI_OK);

    code = ffi_prep_closure_loc(closure, cif, call, (void*) off, ret);
    assert(code == FFI_OK);

    _handles.push_back(handle);

    return reinterpret_cast<intptr_t>(ret);
  }
}
