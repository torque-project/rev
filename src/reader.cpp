#include "reader.hpp"

// #include "rt/core.hpp"
#include "values.hpp"
#include "core.hpp"

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <vector>

using namespace rev;
using namespace rev::rdr;

enum class parse_state_t {
  pass,
  skip,
  fail
};

typedef std::tuple<parse_state_t, value_t::p> result_t;

typedef std::vector<value_t::p> values_t;

typedef std::function<result_t (std::istream&)> macro_t;
typedef std::map<char, macro_t> macros_t;

typedef std::function<result_t (std::istream&)> pass_t;
typedef std::vector<pass_t> passes_t;

typedef std::function<
    value_t::p (const std::string&)
  > ctor_t;

typedef std::function<
    value_t::p (const values_t&)
  > from_list_t;

template<typename T>
macro_t  balanced_form(char until);
macro_t  expand(const macro_t& m);
macro_t  wrap(const char*);
result_t read_or_skip(std::istream& in);
result_t conditional(std::istream& in);
result_t drop(std::istream& in);
result_t meta(std::istream& in);
result_t syntax_quote(std::istream& in);
result_t unquote(std::istream& in);
result_t unbalanced_error(std::istream& in);
result_t read_char(std::istream& in);
result_t read_string(std::istream& in);
result_t read_symbol(std::istream& in);
result_t read_keyword(std::istream& in);
result_t consult_table(std::istream& in);
void     skip_white_space(std::istream& in);
void     skip_comment(std::istream& in);

static sym_t::p QUOTE   = sym_t::intern("quote");
static sym_t::p UNQUOTE = sym_t::intern("unquote");
static sym_t::p SPLICE  = sym_t::intern("unquote-splicing");
static sym_t::p SEQ     = sym_t::intern("seq");
static sym_t::p APPLY   = sym_t::intern("apply");
static sym_t::p LIST    = sym_t::intern("list");
static sym_t::p VECTOR  = sym_t::intern("vector");
static sym_t::p HMAP    = sym_t::intern("hash-map");
static sym_t::p CONCAT  = sym_t::intern("concat");

static macros_t extensions(
  {{'{', balanced_form<set_t>('}')},
   {'?', conditional},
   {'_', drop}});

static macros_t macros(
  {{'(',  balanced_form<list_t>(')')},
   {'[',  balanced_form<vector_t>(']')},
   {'{',  balanced_form<map_t>('}')},
   {'\\', read_char},
   {'\"', read_string},
   // before loading the keyword name space, we treat keywords as regular
   // symbols so that name(kw) returns a string that doesn't contain the
   // colon like name would on a keyword
   {':',  read_keyword},
   {'@',  wrap("deref")},
   {'\'', wrap("quote")},
   {'`',  syntax_quote},
   {'~',  unquote},
   {'^',  meta},
   {'#',  consult_table},
   {')',  unbalanced_error},
   {']',  unbalanced_error}});

inline result_t pass(const value_t::p& v) {
  return std::make_tuple(parse_state_t::pass, v);
}

inline result_t skip() {
  return std::make_tuple(parse_state_t::skip, nullptr);
}

inline result_t fail() {
  return std::make_tuple(parse_state_t::fail, value_t::p());
}

macro_t list_reader(char until, const from_list_t& ctor) {
  return [=](std::istream& in) {

    values_t objs;
    bool terminate = false;

    while (!terminate) {
      // skip whitespace before reading next list element
      // to catch cases like:
      // (list a b
      //   )
      skip_white_space(in);
      skip_comment(in);

      char c = in.peek();

      if (c == std::char_traits<char>::eof()) {
        throw eos();
      }
      else if (c == until) {
        in.get();
        terminate = true;
      }
      else {
        auto result = read_or_skip(in);
        if (std::get<0>(result) != parse_state_t::skip) {
          objs.push_back(std::get<1>(result));
        }
      }
    }

    return pass(ctor(objs));
  };
}

template<typename T>
macro_t balanced_form(char until) {
  return list_reader(until, [](const values_t& objs) {
      return T::from_std(objs);
  });
}

macro_t wrap(const char* s) {

  std::string sym(s);

  return [sym](std::istream& in) {

    values_t forms({
      sym_t::intern(sym),
        read(in)
      });

    return pass(list_t::from_std(forms));
  };
}

result_t conditional(std::istream& in) {

  static const keyw_t::p REV = keyw_t::intern("rev");
  static const keyw_t::p DEF = keyw_t::intern("default");

  auto branches = imu::partition<list_t::p>(2, as<list_t>(read(in)));

  value_t::p form = nullptr, def = nullptr;
  while (!imu::is_empty(branches)) {
    auto kv = as<list_t>(imu::first(branches));
    auto k  = as<keyw_t>(imu::first(kv));

    if (k == REV) {
      form = *imu::second(kv);
      break;
    }
    else if (k == DEF) {
      def = *imu::second(kv);
    }
    branches = imu::rest(branches);
  }

  if (form || def) {
    return form ? pass(form) : pass(def);
  }

  return skip();
}

result_t drop(std::istream& in) {
  // read two forms and return the second
  read(in);
  return skip();
}

result_t meta(std::istream& in) {
  // read two forms and return the second
  auto meta = read(in);
  auto form = read(in);

  map_t::p m;

  if (auto sym = as_nt<keyw_t>(meta)) {
    m = imu::assoc(imu::nu<map_t>(), sym, sym_t::true_);
  }
  else if (auto m2 = as_nt<map_t>(meta)) {
    m = m2;
  }

  if (m) {
    void* args[] = {(void*) form, (void*) m};
    form = protocol_t::dispatch(protocol_t::withmeta, 0, args, 2);
  }

  return pass(form);
}

value_t::p do_syntax_quote(const value_t::p& form);

template<typename S>
list_t::p expand_seq(const S& s) {

  std::vector<value_t::p> out;

  imu::for_each([&](const value_t::p& v) {
    auto lst = as_nt<list_t>(v);
    auto sym = as_nt<sym_t>(imu::first(lst));
    if (sym && sym == UNQUOTE) {
      out.push_back(list_t::factory(LIST, *imu::second(lst)));
    }
    else if (sym && sym == SPLICE) {
      out.push_back(*imu::second(lst));
    }
    else {
      out.push_back(list_t::factory(LIST, do_syntax_quote(v)));
    }
  }, s);

  return list_t::from_std(out);
}

value_t::p do_syntax_quote(const value_t::p& form) {
  if (auto sym = as_nt<sym_t>(form)) {
    auto mangled = sym;
    if (sym->name().back() != '#') {
      mangled = qualify(sym);
    }
    // TODO: handle # symbols
    return list_t::factory(QUOTE, mangled);
  }
  else if (auto lst = as_nt<list_t>(form)) {
    if (auto s = imu::seq(lst)) {
      return list_t::factory(SEQ, imu::conj(expand_seq(s), CONCAT));
    }
    else {
      return imu::nu<list_t>();
    }
  }
  else if (auto vec = as_nt<vector_t>(form)) {
    return list_t::factory(APPLY, VECTOR, imu::conj(expand_seq(seq(vec)), CONCAT));
  }
  else if (auto map = as_nt<map_t>(form)) {
    return list_t::factory(APPLY, HMAP, imu::conj(expand_seq(seq(vec)), CONCAT));
  }
  else if (is<int_t>(form) || is<string_t>(form)) {
    return form;
  }

  return list_t::factory(QUOTE, form);
}

result_t syntax_quote(std::istream& in) {
  return pass(do_syntax_quote(read(in)));
}

result_t unquote(std::istream& in) {

  std::string quote =
    (in.peek() == '@') ? (in.get(), "unquote-splicing") : "unquote";

  values_t forms({
    sym_t::intern(quote),
    read(in)
  });

  return pass(list_t::from_std(forms));
}

std::string escape(const std::string& s) {

  std::string out;

  auto i = s.begin();
  auto j = i;

  while (i != s.end()) {
    if (*(i++) == '\\') {
      if (i == s.end()) {
        throw std::runtime_error(
          "Incomplete escape sequence at thend of string");
      }
      out.append(j, i-1);
      switch(*i) {
      case '0':  out += '\0'; break;
      case 'n':  out += '\n'; break;
      case 'r':  out += '\r'; break;
      case 't':  out += '\t'; break;
      case '\\': out += '\\'; break;
      default:
        throw std::runtime_error(
          std::string("Unsupported escape sequence: \\") + *i);
      }
      j = ++i;
    }
  }

  out.append(j, i);

  return out;
}

bool is_token(char c) {
  return
    (c != std::char_traits<char>::eof())
    && (!isspace(c))
    && (c != ')') && (c != '}') && (c != ']')
    //&& (macros.find(c) == macros.end())
    ;
}

result_t read_char(std::istream& in) {
  std::string buf;

  char ch = in.peek();

  if (is_token(ch)) {
    do {
      in.get();
      buf += ch;
      ch   = in.peek();
    } while (in.good() && is_token(ch));
  }
  else {
    in.get();
    buf += ch;
  }

  if (buf.size() == 1) {
    return pass(imu::nu<int_t>(buf[0]));
  }
  else {
    // TODO: interpret char token
  }

  return fail();
}

macro_t string_reader(const ctor_t& ctor) {
  return [=](std::istream& in) {
    std::string str;
    std::getline(in, str, '\"');

    if (in.eof()) {
      throw unbalanced('\"');
    }

    return pass(ctor(escape(str)));
  };
}

result_t read_string(std::istream& in) {
  static const auto read = string_reader([](const std::string& s) {
      return string_t::intern(s);
  });
  return read(in);
}

result_t try_macros(std::istream& in, const macros_t& macs) {

  result_t res = fail();

  auto iter = macs.find(in.peek());
  if (iter != macs.end()) {
    in.get();
    res = iter->second(in);
  }

  return res;
}

result_t unbalanced_error(std::istream& in) {
  throw unbalanced(in.peek());
}

result_t consult_table(std::istream& in) {
  return try_macros(in, extensions);
}

result_t try_default_macros(std::istream& in) {
  return try_macros(in, macros);
}

macro_t symbol_reader(const ctor_t& ctor) {
  return [=](std::istream& in){
    char c = in.peek();
    if (is_token(c)) {

      std::string sym;

      do {

        sym += in.get();
        c = in.peek();

      } while(is_token(c));

      return pass(ctor(sym));
    }

    return fail();
  };
}

result_t read_symbol(std::istream& in) {
  static const auto read = symbol_reader([](const std::string& s){
      if (s == "nil") {
        return sym_t::p();
      }
      return sym_t::intern(s);
    });
  return read(in);
}

result_t read_keyword(std::istream& in) {
  static const auto read = symbol_reader([](const std::string& s){
      if (s.size() > 0 && s[0] == ':') {
        return keyw_t::intern(ns()->name() + "/"  + s.substr(1));
      }
      return keyw_t::intern(s);
    });
  return read(in);
}

result_t try_number(std::istream& in) {

  std::string s;

  if (!isdigit(in.peek())) {
    if (in.peek() == '-') {
      s += in.get();
      if (!isdigit(in.peek())) {
        in.putback('-');
        return fail();
      }
    }
    else {
      return fail();
    }
  }

  do {
    s += in.get();
  } while (isalnum(in.peek()));

  return pass(imu::nu<int_t>(std::stoll(s, nullptr, 0)));
}

void skip_white_space(std::istream& in) {

  char c = in.peek();
  while(isspace(c)) {

    in.get();
    c = in.peek();
  }
}

void skip_comment(std::istream& in) {

  while (in.peek() == ';') {
    do {
      in.get();
    } while (in.peek() != '\n');
    skip_white_space(in);
  }
}

result_t read_or_skip(std::istream& in) {

  skip_white_space(in);
  skip_comment(in);

  if (in.eof()) {
    return pass(nullptr);
  }

  passes_t passes({
      &try_number
    , &try_default_macros
    , &read_symbol});

  result_t res = fail();

  for(auto x : passes) {
    res = x(in);
    if(std::get<0>(res) != parse_state_t::fail) {
      break;
    }
  }

  if (std::get<0>(res) == parse_state_t::fail) {
    throw std::runtime_error("Unable to parse form");
  }

  return res;
}

value_t::p rev::rdr::read(std::istream& in) {
  auto result = read_or_skip(in);
  if (std::get<0>(result) == parse_state_t::pass) {
    return std::get<1>(result);
  }
  return nullptr;
}

value_t::p rev::rdr::read(const std::string& s) {
  std::stringstream in(s);
  return read(in);
}

void set_macro(char begin, const macro_t& reader) {
  macros[begin] = [=](std::istream& in) {
    return reader(in);
  };
}

void push_macro_table(char begin, const macro_t& reader) {
  extensions[begin] = reader;
}
