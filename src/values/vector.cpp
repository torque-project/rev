#include "../adapter.hpp"
#include "../values.hpp"

namespace rev {

  value_t::p Vector_Printable_str(value_t::p self) {
    auto v = as<vector_t>(self);
    std::string s = "[";
    if (auto fst = imu::first(v)) {
      s += str(*fst);
      imu::for_each([&](const value_t::p& v) {
          auto str = rev::str(v);
          s += " " + str;
        }, imu::rest(v));
    }
    s += "]";
    return imu::nu<string_t>(s);
  }

  value_t::p Vector_Seqable_seq(value_t::p self) {
    auto v = as<vector_t>(self);
    if (imu::count(v) > 0) {
      return imu::nu<seq_adapter_t<vector_t>>(v);
    }
    return nullptr;
  }

  value_t::p Vector_Collection_conj(value_t::p self, value_t::p o) {
    return imu::conj(as<vector_t>(self), o);
  }

  value_t::p Vector_Associative_contains_key(value_t::p self, value_t::p k) {
    return sym_t::false_;
  }

  value_t::p Vector_Associative_assoc(
    value_t::p self, value_t::p k, value_t::p val) {

    auto v = as<vector_t>(self);
    auto n = as<int_t>(k);
    return imu::assoc(v, n->value, val);
  }

  value_t::p Vector_IIndexed_nth2(value_t::p self, value_t::p n) {
    return as<vector_t>(self)->nth(as<int_t>(n)->value);
  }

  value_t::p Vector_IIndexed_nth3(value_t::p self, value_t::p n, value_t::p d) {
    auto i = as<int_t>(n)->value;
    auto v = as<vector_t>(self);
    if (i >= 0 && i < imu::count(v)) {
      return v->nth(i);
    }
    return d;
  }

  value_t::p Vector_Lookup_lookup2(value_t::p s, value_t::p k) {
    return Vector_IIndexed_nth2(s, k);
  }

  value_t::p Vector_Lookup_lookup3(value_t::p s, value_t::p k, value_t::p d) {
    return Vector_IIndexed_nth3(s, k, d);
  }

  value_t::p VectorSeq_Seqable_seq(value_t::p self) {
    return self;
  }

  value_t::p VectorSeq_Seq_first(value_t::p self) {
    auto seq = as<seq_adapter_t<vector_t>>(self)->seq();
    if (!imu::is_empty(seq)) {
      return seq->first();
    }
    return nullptr;
  }

  value_t::p VectorSeq_Seq_rest(value_t::p self) {
    auto seq = as<seq_adapter_t<vector_t>>(self)->seq();
    if (!imu::is_empty(seq)) {
      if (auto rest = seq->rest()) {
        return imu::nu<seq_adapter_t<vector_t>>(rest);
      }
    }
    return nullptr;
  }

  value_t::p VectorSeq_Next_next(value_t::p self) {
    auto seq = as<seq_adapter_t<vector_t>>(self)->seq();
    if (!imu::is_empty(seq)) {
      if (auto rest = seq->rest()) {
        return imu::nu<seq_adapter_t<vector_t>>(rest);
      }
    }
    return nullptr;
  }

  value_t::p Vector_MapEntry_key(value_t::p self) {
    return as<vector_t>(self)->nth(0);
  }

  value_t::p Vector_MapEntry_val(value_t::p self) {
    return as<vector_t>(self)->nth(1);
  }

  value_t::p Vector_Equiv_equiv(value_t::p self, value_t::p other) {
    auto s = imu::seq(as<vector_t>(self));
    return imu::seqs::equiv(s, rt_seq_t::seq(other), equal_to()) ?
      sym_t::true_ : sym_t::false_;
  }

  struct type_t::impl_t Vector_printable[] = {
    {(intptr_t) Vector_Printable_str, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_seqable[] = {
    {(intptr_t) Vector_Seqable_seq, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_coll[] = {
    {0, (intptr_t) Vector_Collection_conj, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_associative[] = {
    {0, (intptr_t) Vector_Associative_contains_key, 0, 0, 0, 0, 0, 0},
    {0, 0, (intptr_t) Vector_Associative_assoc, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_iindexed[] = {
    {0,
     (intptr_t) Vector_IIndexed_nth2,
     (intptr_t) Vector_IIndexed_nth3,
     0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_lookup[] = {
    {0,
     (intptr_t) Vector_Lookup_lookup2,
     (intptr_t) Vector_Lookup_lookup3,
     0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_mapentry[] = {
    {(intptr_t) Vector_MapEntry_key, 0, 0, 0, 0, 0, 0, 0},
    {(intptr_t) Vector_MapEntry_val,  0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Vector_equiv[] = {
    {0, (intptr_t) Vector_Equiv_equiv, 0, 0, 0, 0, 0, 0},
  };

  struct type_t::impl_t VectorSeq_seqable[] = {
    {(intptr_t) VectorSeq_Seqable_seq, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t VectorSeq_seq[] = {
    {(intptr_t) VectorSeq_Seq_first, 0, 0, 0, 0, 0, 0, 0},
    {(intptr_t) VectorSeq_Seq_rest,  0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t VectorSeq_next[] = {
    {(intptr_t) VectorSeq_Next_next, 0, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Vector_methods[] = {
    {protocol_t::str,         Vector_printable},
    {protocol_t::seqable,     Vector_seqable},
    {protocol_t::ivector,     nullptr},
    {protocol_t::coll,        Vector_coll},
    {protocol_t::associative, Vector_associative},
    {protocol_t::indexed,     Vector_iindexed},
    {protocol_t::equiv,       Vector_equiv},
    {protocol_t::lookup,      Vector_lookup},
    {protocol_t::mapentry,    Vector_mapentry}
  };

  struct type_t::ext_t VectorSeq_methods[] = {
    {protocol_t::seqable, VectorSeq_seqable},
    {protocol_t::seq,     VectorSeq_seq},
    {protocol_t::next,    VectorSeq_next}
  };

  static const uint64_t vec_size =
    sizeof(Vector_methods) / sizeof(Vector_methods[0]);

  static const uint64_t seq_size =
    sizeof(VectorSeq_methods) / sizeof(VectorSeq_methods[0]);

  template<>
  type_t value_base_t<vector_tag_t>::prototype(
    "PersistentVector.0", Vector_methods, vec_size);

  template<>
  type_t value_base_t<seq_adapter_t<vector_t>>::prototype(
    "VectorSeq.0", VectorSeq_methods, seq_size);

  template<>
  type_t* prototype<vector_t>() {
    return &value_base_t<vector_tag_t>::prototype;
  }

  template<>
  type_t* prototype<seq_adapter_t<vector_t>>() {
    return &value_base_t<seq_adapter_t<vector_t>>::prototype;
  }
}

extern "C" {

  const rev::type_t* torque_lang_vector_PersistentVector =
    &rev::vector_t::prototype;

  rev::value_t::p torque_lang_vector_empty = imu::nu<rev::vector_t>();
}
