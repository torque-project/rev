#include "../values.hpp"
#include "../core.hpp"
#include "../compiler.hpp"

#include <cassert>

#include <ffi.h>

namespace rev {

  template<>
  type_t value_base_t<type_value_t>::prototype("Type");

  struct type_t::native_handle_t {
    ffi_cif*     cif;
    ffi_closure* closure;
    ffi_type**   args;

    struct address_t {
      int64_t off;
      int64_t length;
    } address;

    ~native_handle_t() {
      ffi_closure_free(closure);
      delete args;
      delete cif;
    }
  };

  void call(ffi_cif *cif, void* ret, void* ptrs[], void* a) {

    auto pad     = (value_t**) ret;
    auto self    = *((value_t**) ptrs[0]);
    auto address = (type_t::native_handle_t::address_t*) a;
    auto code    = self->type->_code + (int64_t) fn_t::offset(address->off);
    auto to      = code + address->length;
    auto stack   = fn_t::stack_space(address->off, cif->nargs);

    value_t::p args[cif->nargs+1];
    for (int i=cif->nargs; i>=1; --i) {
      args[i] = *((value_t**) ptrs[i-1]);
    }
    args[0] = self;

    *pad = call(code, to, stack, args, cif->nargs+1);
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

    handle->address.off    = off;
    handle->address.length = 0;

    code = ffi_prep_closure_loc(closure, cif, call, (void*) &handle->address, ret);
    assert(code == FFI_OK);

    _handles.push_back(handle);

    return reinterpret_cast<intptr_t>(ret);
  }

  void type_t::finalize(int64_t address) {

    _code = address;

    for (auto& handle : _handles) {
      auto length = compute_fn_length(_code + fn_t::offset(handle->address.off));
      handle->address.length = length;
    }
  }
}
