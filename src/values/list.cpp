#include "../values.hpp"

namespace rev {

  value_t::p List_Seqable_seq(value_t::p self) {
    return self;
  }

  value_t::p List_Seq_first(value_t::p self) {
    return as<list_t>(self)->first();
  }

  value_t::p List_Seq_rest(value_t::p self) {
    return as<list_t>(self)->rest();
  }

  value_t::p List_Next_next(value_t::p self) {
    return as<list_t>(self)->rest();
  }

  struct type_t::impl_t List_seqable[] = {
    {0, (intptr_t) List_Seqable_seq, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t List_seq[] = {
    {0, (intptr_t) List_Seq_first, 0, 0, 0, 0, 0, 0},
    {0, (intptr_t) List_Seq_rest,  0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t List_next[] = {
    {0, (intptr_t) List_Next_next, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t List_methods[] = {
    {protocol_t::alist,   nullptr},
    {protocol_t::seqable, List_seqable},
    {protocol_t::seq,     List_seq},
    {protocol_t::next,    List_next}
  };

  static const uint64_t size = sizeof(List_methods) / sizeof(List_methods[0]);

  template<>
  type_t value_base_t<list_tag_t>::prototype("List.0", List_methods, size);
}
