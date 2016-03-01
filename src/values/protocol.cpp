#include "../values.hpp"

#include <cassert>
#include <sstream>

#include <ffi.h>

namespace rev {

  uint64_t protocol_t::ids = 0;

  template<>
  type_t value_base_t<protocol_t>::prototype("Protocol");

  value_t::p protocol_t::dispatch(
    uint32_t id, uint32_t m, void* args[], uint32_t n) {

    assert(n >= 1);

    ffi_cif   cif;
    ffi_type* types[n];
    void*     values[n];

    for (int i=0; i<n; ++i) {
      types[i]  = &ffi_type_pointer;
      values[i] = &(args[i]);
    }

    auto self = (value_t::p) args[0];
    auto type = self->type;
    
    if (auto sym = as_nt<sym_t>(self)) {
      std::cout << sym->name() << std::endl;
    }
    void* ret    = nullptr;
    void  (*f)() = nullptr;

    for (int i=0; i<type->_num_ext; ++i) {
      if (type->_methods[i].id == id) {
        f = (void (*)()) type->_methods[i].impls[m].arities[n];
        break;
      }
    }

    if (f) {
      ffi_prep_cif(&cif, FFI_DEFAULT_ABI, n, &ffi_type_pointer, types);
      ffi_call(&cif, f, &ret, values);
      return (value_t::p) ret;
    }
    else {
      std::stringstream ss;
      ss << "Method not implemented: " << id << " " << m;
      throw std::runtime_error(ss.str());
    }
  }
}
