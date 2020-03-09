#include "../adapter.hpp"
#include "../values.hpp"

namespace rev {

  value_t::p List_Printable_str(value_t::p self) {
    auto lst = as<list_t>(self);
    std::string s = "(";
    if (auto fst = imu::first(lst)) {
      s += str(*fst);
      imu::for_each([&](const value_t::p& v) {
          auto str = rev::str(v);
          s += " " + str;
        }, imu::rest(lst));
    }
    s += ")";
    return imu::nu<string_t>(s);
  }

  struct type_t::impl_t List_printable[] = {
    {(intptr_t) List_Printable_str, 0, 0, 0, 0, 0, 0, 0}
  };

  value_t::p List_Coll_conj(value_t::p self, value_t::p x) {
    return imu::conj(as<list_t>(self), x);
  }

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

  value_t::p List_Counted_count(value_t::p self) {
    return imu::nu<int_t>(as<list_t>(self)->count());
  }

  value_t::p List_Equiv_equiv(value_t::p self, value_t::p other) {
    auto s = imu::seq(as<list_t>(self));
    return imu::seqs::equiv(s, rt_seq_t::seq(other), equal_to()) ?
      sym_t::true_ : sym_t::false_;
  }

  struct type_t::impl_t List_coll[] = {
    {(intptr_t) List_Coll_conj, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t List_seqable[] = {
    {(intptr_t) List_Seqable_seq, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t List_seq[] = {
    {(intptr_t) List_Seq_first, 0, 0, 0, 0, 0, 0, 0},
    {(intptr_t) List_Seq_rest, 0,  0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t List_next[] = {
    {(intptr_t) List_Next_next, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t List_counted[] = {
    {(intptr_t) List_Counted_count, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t List_equiv[] = {
    {0, (intptr_t) List_Equiv_equiv, 0, 0, 0, 0, 0, 0},
  };

  struct type_t::ext_t List_methods[] = {
    {protocol_t::alist,   nullptr},
    {protocol_t::str,     List_printable},
    {protocol_t::coll,    List_coll},
    {protocol_t::seqable, List_seqable},
    {protocol_t::seq,     List_seq},
    {protocol_t::next,    List_next},
    {protocol_t::counted, List_counted},
    {protocol_t::equiv,   List_equiv}
  };

  static const uint64_t size = sizeof(List_methods) / sizeof(List_methods[0]);

  template<>
  type_t value_base_t<list_tag_t>::prototype("List.0", List_methods, size);
}

extern "C" {
  rev::value_t::p torque_lang_list_empty = imu::nu<rev::list_t>();
}
