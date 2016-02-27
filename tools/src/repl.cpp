#include "core.hpp"
#include "reader.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>

#include <readline/readline.h>
#include <readline/history.h>

#include <stdlib.h>

void print(const rev::value_t::p& v) {
  if (auto lst = rev::as_nt<rev::list_t>(v)) {
    std::cout << "(";
    if (auto fst = imu::first(lst)) {
      print(*fst);
      imu::for_each([&](const rev::value_t::p& v) {
          std::cout << " "; print(v);
        }, imu::rest(lst));
      std::cout << ")";
    }
  }
  else if (auto sym = rev::as_nt<rev::sym_t>(v)) {
    std::cout << sym->name();
  }
  else if (auto n = rev::as_nt<rev::int_t>(v)) {
    std::cout << n->value;
  }
  else {
    std::cout << "#{" << v->type->name() << ": " << v << "}" << std::endl;
  }
}

int main(int argc, char** argv) {

#ifdef _DEBUG
  std::cout << "Starting repl in debug mode" << std::endl;
#endif

  read_history(".repl_history");

  try {

    std::string sources = "";
    if (char* s = getenv("REV_SOURCE_PATH")) {
      sources = s;
    }
    // TODO: make source path configurable via parameter too

    rev::boot(1 << 16, sources);

    char* line = NULL;
    for(;;) {

      std::string prefix = rev::ns()->name();

      line = readline((prefix + "> ").c_str());
      if (!line) {
        break;
      }

      if (strlen(line) == 0) {
        continue;
      }

      try {
        auto form = rev::read(line);

        // save line in history once we've successfully read it
        // this orevents copy/paste gobbledigook from bombing
        // the history
        add_history(line);
        write_history(".repl_history");

        auto result = rev::eval(form);
        print(result);
        std::cout << std::endl;
      }
      catch(rev::rdr::reader_exception& e) {
        std::cout << "Error while reading object: " << e.what() << std::endl;
      }
      catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
      }

      free(line);
    }
  }
  catch(std::exception& e) {
    std::cerr << "Can't boot VM: " << e.what() << std::endl;
  }
}
