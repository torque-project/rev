#include "reader.hpp"

// #include "rt/core.hpp"
#include "values.hpp"

#include <functional>
#include <iostream>
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
result_t drop(std::istream& in);
result_t unquote(std::istream& in);
result_t unbalanced_error(std::istream& in);
result_t read_string(std::istream& in);
result_t read_symbol(std::istream& in);
result_t consult_table(std::istream& in);
void     skip_white_space(std::istream& in);
void     skip_comment(std::istream& in);

static macros_t extensions(
  {{'_', drop}});

static macros_t macros(
  {{'(',  balanced_form<list_t>(')')},
   {'[',  balanced_form<vector_t>(']')},
   {'{',  balanced_form<map_t>('}')},
   {'\"', read_string},
   // before loading the keyword name space, we treat keywords as regular
   // symbols so that name(kw) returns a string that doesn't contain the
   // colon like name would on a keyword
   {':',  read_symbol},
   {'@',  wrap("deref")},
   {'\'', wrap("quote")},
   {'`',  wrap("syntax-quote")},
   {'~',  unquote},
   {'^',  drop},
   {'#',  consult_table},
   {')',  unbalanced_error},
   {']',  unbalanced_error}});

inline result_t pass(const value_t::p& v) {
  return std::make_tuple(parse_state_t::pass, v);
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

result_t drop(std::istream& in) {
  // read two forms and return the second
  read(in);
  return std::make_tuple(parse_state_t::skip, nullptr);
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

bool is_sym_token(char c) {
  return
    (c != std::char_traits<char>::eof())
    && (!isspace(c))
    && (c != ')') && (c != '}') && (c != ']')
    //&& (macros.find(c) == macros.end())
    ;
}

macro_t symbol_reader(const ctor_t& ctor) {
  return [=](std::istream& in){
    char c = in.peek();
    if (is_sym_token(c)) {

      std::string sym;

      do {

        sym += in.get();
        c = in.peek();

      } while(is_sym_token(c));

      if (sym == "nil") {
        return pass(value_t::p());
      }

      return pass(ctor(sym));
    }

    return fail();
  };
}

result_t read_symbol(std::istream& in) {
  static const auto read = symbol_reader([](const std::string& s){
      return sym_t::intern(s);
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
