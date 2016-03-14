#include "../values.hpp"

#include <unordered_map>

namespace rev {

  value_t::p Symbol_Printable_str(value_t::p self) {
    auto sym = as<sym_t>(self);
    if (sym->has_ns()) {
      return imu::nu<string_t>(sym->ns() + "/" + sym->name());
    }
    return imu::nu<string_t>(sym->name());
  }

  value_t::p Symbol_WithMeta_withmeta(value_t::p self, value_t::p m) {
    // FIXME: symbols are interned by default, so this would set the meta
    // of all symbols with this name. create a copy instead. we have to
    // implement equiv first though
    self->set_meta(m);
    return self;
  }

  value_t::p Symbol_Named_name(value_t::p self) {
    return imu::nu<string_t>(as<sym_t>(self)->name());
  }

  value_t::p Symbol_Named_namespace(value_t::p self) {
    auto sym = as<sym_t>(self);
    if (sym->has_ns()) {
      return imu::nu<string_t>(sym->ns());
    }
    return nullptr;
  }

  value_t::p Keyword_Printable_str(value_t::p self) {
    auto kw = as<keyw_t>(self);
    if (kw->has_ns()) {
      return imu::nu<string_t>(":" + kw->ns() + "/" + kw->name());
    }
    return imu::nu<string_t>(":" + kw->name());
  }

  value_t::p Keyword_IFn_invoke2(value_t::p self, value_t::p m) {
    if (m) {
      void* args[] = {(void*) m, (void*) self};
      return protocol_t::dispatch(protocol_t::lookup, 0, args, 2);
    }
    return nullptr;
  }

  value_t::p Keyword_IFn_invoke3(value_t::p self, value_t::p m, value_t::p d) {
    if (m) {
      void* args[] = {(void*) m, (void*) self, (void*) d};
      return protocol_t::dispatch(protocol_t::lookup, 0, args, 3);
    }
    return d;
  }

  struct type_t::impl_t Symbol_printable[] = {
    {0, (intptr_t) Symbol_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Symbol_named[] = {
    {0, (intptr_t) Symbol_Named_name, 0, 0, 0, 0, 0, 0},
    {0, (intptr_t) Symbol_Named_namespace, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Symbol_withmeta[] = {
    {0, 0, (intptr_t) Symbol_WithMeta_withmeta, 0, 0, 0, 0, 0}
  };

  // struct type_t::impl_t Symbol_equiv[] = {
  //   {0, 0, (intptr_t) Symbol_Equiv_equiv, 0, 0, 0, 0, 0}
  // };

  struct type_t::ext_t Symbol_methods[] = {
    {protocol_t::str,      Symbol_printable},
    {protocol_t::withmeta, Symbol_withmeta},
    {protocol_t::named,    Symbol_named}
    //    {protocol_t::equiv, Symbol_equiv}
  };

  struct type_t::impl_t Keyword_printable[] = {
    {0, (intptr_t) Keyword_Printable_str, 0, 0, 0, 0, 0, 0}
  };

  struct type_t::impl_t Keyword_ifn[] = {
    {0, 0,
     (intptr_t) Keyword_IFn_invoke2,
     (intptr_t) Keyword_IFn_invoke3,
     0, 0, 0, 0}
  };

  // struct type_t::impl_t Keyword_equiv[] = {
  //   {0, 0, (intptr_t) Keyword_Equiv_equiv, 0, 0, 0, 0, 0}
  // };

  struct type_t::ext_t Keyword_methods[] = {
    {protocol_t::str,   Keyword_printable},
    {protocol_t::ifn,   Keyword_ifn}
    //{protocol_t::equiv, Keyword_equiv}
  };

  static const uint64_t sym_size =
    sizeof(Symbol_methods) / sizeof(Symbol_methods[0]);
  static const uint64_t kw_size  =
    sizeof(Keyword_methods) / sizeof(Keyword_methods[0]);

  template<>
  type_t value_base_t<sym_t>::prototype("Symbol.0", Symbol_methods, sym_size);

  template<>
  type_t value_base_t<keyw_t>::prototype("Keyword.0", Keyword_methods, kw_size);

  template<>
  sym_base_t<sym_t>::p sym_base_t<sym_t>::intern(const std::string& fqn) {

    static std::unordered_map<std::string, sym_t::p> cache;

    auto& sym = cache[fqn];
    return sym ? sym : (sym = imu::nu<sym_t>(fqn));
  }

  template<>
  sym_base_t<sym_t>::p sym_base_t<sym_t>::intern(
    const std::string& ns, const std::string& name) {
    return intern(ns + "/" + name);
  }

  template<>
  sym_base_t<keyw_t>::p sym_base_t<keyw_t>::intern(const std::string& fqn) {

    static std::unordered_map<std::string, keyw_t::p> cache;

    auto& k = cache[fqn];
    return k ? k : (k = imu::nu<keyw_t>(fqn));
  }

  sym_t::p sym_t::true_  = sym_t::intern("true");
  sym_t::p sym_t::false_ = sym_t::intern("false");
}
