#include "../values.hpp"

namespace rev {

  value_t::p Int_Equiv_equiv(value_t::p self, value_t::p other) {
    auto i = as_nt<int_t>(other);
    return (i && as<int_t>(self)->value == i->value) ?
      sym_t::true_ : sym_t::false_;
  }

  struct type_t::impl_t Int_equiv[] = {
    {0, 0, (intptr_t) Int_Equiv_equiv, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Int_methods[] = {
    {protocol_t::equiv, Int_equiv}
  };

  static const uint64_t size = sizeof(Int_methods) / sizeof(Int_methods[0]);

  template<>
  type_t value_base_t<int_t>::prototype("Integer.0", Int_methods, size);
}
