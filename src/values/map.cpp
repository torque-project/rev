#include "../values.hpp"
#include "../adapter.hpp"

namespace rev {

  value_t::p Map_Printable_str(value_t::p self) {
    auto m = as<map_t>(self);
    std::string s = "{";
    if (auto fst = imu::first(m)) {
      auto a = rev::str(imu::first(*fst));
      auto b = rev::str(imu::second(*fst));
      s += a->data() + " " + b->data();
      imu::for_each([&](const map_t::value_type& v) {
          auto a = rev::str(imu::first(v));
          auto b = rev::str(imu::second(v));
          s += " " + a->data() + " " + b->data();
        }, imu::rest(m));
    }
    s += "}";
    return imu::nu<string_t>(s);
  }

  value_t::p Map_Seqable_seq(value_t::p self) {
    return imu::nu<seq_adapter_t<map_t>>(as<map_t>(self));
  }

  value_t::p Map_Associative_contains_key(value_t::p self, value_t::p n) {
    return imu::get(as<map_t>(self), n) ? sym_t::true_ : sym_t::false_;
  }

  value_t::p Map_Associative_assoc(value_t::p s, value_t::p k, value_t::p v) {
    return imu::assoc(as<map_t>(s), k, v);
  }

  value_t::p Map_IMap_dissoc(value_t::p s, value_t::p k) {
    return imu::dissoc(as<map_t>(s), k);
  }

  value_t::p Map_Lookup_lookup2(value_t::p s, value_t::p k) {
    if (auto v = imu::get(as<map_t>(s), k)) {
      return *v;
    }
    return nullptr;
  }

  value_t::p Map_Lookup_lookup3(value_t::p s, value_t::p k, value_t::p d) {
    return imu::get(as<map_t>(s), k, d);
  }

  value_t::p Map_Collection_conj(value_t::p self, value_t::p x) {
    if (x) {
      if (auto v = as_nt<vector_t>(x)) {
        return imu::assoc(as<map_t>(self), imu::nth(v, 0), imu::nth(v, 1));
      }
      else if (protocol_t::satisfies(protocol_t::mapentry, x)) {
        rt_vec_t v(x);
        return imu::assoc(as<map_t>(self), v.key(), v.val());
      }
      else {
        return imu::reduce([](const map_t::p& m, const value_t::p& y) {
            if (y) {
              rt_vec_t v(y);
              return imu::assoc(m, v.key(), v.val());
            }
            return m;
          }, as<map_t>(self), rt_seq_t::seq(x));
      }
    }
    return self;
  }

  value_t::p Map_WithMeta_with_meta(value_t::p self, value_t::p meta) {
    auto m = imu::nu<map_t>(*as<map_t>(self));
    m->set_meta(meta);
    return m;
  }

  value_t::p Map_Counted_count(value_t::p self) {
    return imu::nu<int_t>(as<map_t>(self)->count());
  }

  value_t::p MapSeq_Seqable_seq(value_t::p self) {
    return self;
  }

  value_t::p MapSeq_Seq_first(value_t::p self) {
    auto seq = as<seq_adapter_t<map_t>>(self)->seq();
    if (!imu::is_empty(seq)) {
      auto kv = seq->first();
      return vector_t::factory(imu::first(kv), imu::second(kv));
    }
    return nullptr;
  }

  value_t::p MapSeq_Seq_rest(value_t::p self) {
    auto seq = as<seq_adapter_t<map_t>>(self)->seq();
    if (!imu::is_empty(seq)) {
      if (auto rest = seq->rest()) {
        return imu::nu<seq_adapter_t<map_t>>(rest);
      }
    }
    return nullptr;
  }

  value_t::p MapSeq_Next_next(value_t::p self) {
    auto seq = as<seq_adapter_t<map_t>>(self)->seq();
    if (!imu::is_empty(seq)) {
      if (auto rest = seq->rest()) {
        return imu::nu<seq_adapter_t<map_t>>(rest);
      }
    }
    return nullptr;
  }

  struct type_t::impl_t Map_printable[] = {
    {0, (intptr_t) Map_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Map_seqable[] = {
    {0, (intptr_t) Map_Seqable_seq, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Map_collection[] = {
    {0, 0,
     (intptr_t) Map_Collection_conj,
     0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Map_with_meta[] = {
    {0, 0,
     (intptr_t) Map_WithMeta_with_meta,
     0, 0, 0, 0}
  };

  struct type_t::impl_t Map_associative[] = {
    {0, 0,
     (intptr_t) Map_Associative_contains_key,
     0, 0, 0, 0},
    {0, 0, 0,
     (intptr_t) Map_Associative_assoc,
     0, 0, 0, 0}
  };

  struct type_t::impl_t Map_imap[] = {
    {0, 0,
     (intptr_t) Map_IMap_dissoc,
     0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Map_lookup[] = {
    {0, 0,
     (intptr_t) Map_Lookup_lookup2,
     (intptr_t) Map_Lookup_lookup3,
     0, 0, 0, 0}
  };

  struct type_t::impl_t Map_counted[] = {
    {0, (intptr_t) Map_Counted_count, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t MapSeq_seqable[] = {
    {0, (intptr_t) MapSeq_Seqable_seq, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t MapSeq_seq[] = {
    {0, (intptr_t) MapSeq_Seq_first, 0, 0, 0, 0, 0, 0},
    {0, (intptr_t) MapSeq_Seq_rest,  0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t MapSeq_next[] = {
    {0, (intptr_t) MapSeq_Next_next, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::ext_t Map_methods[] = {
    {protocol_t::str,         Map_printable},
    {protocol_t::seqable,     Map_seqable},
    {protocol_t::associative, Map_associative},
    {protocol_t::imap,        Map_imap},
    {protocol_t::coll,        Map_collection},
    {protocol_t::lookup,      Map_lookup},
    {protocol_t::withmeta,    Map_with_meta},
    {protocol_t::counted,     Map_counted}
  };

  struct type_t::ext_t MapSeq_methods[] = {
    {protocol_t::seqable, MapSeq_seqable},
    {protocol_t::seq,     MapSeq_seq},
    {protocol_t::next,    MapSeq_next}
  };

  static const uint64_t map_size =
    sizeof(Map_methods) / sizeof(Map_methods[0]);

  static const uint64_t seq_size =
    sizeof(MapSeq_methods) / sizeof(MapSeq_methods[0]);

  template<>
  type_t value_base_t<map_tag_t>::prototype(
    "PersistentArrayMap.0", Map_methods, map_size);

  template<>
  type_t value_base_t<seq_adapter_t<map_t>>::prototype(
    "MapSeq.0", MapSeq_methods, seq_size);
}
