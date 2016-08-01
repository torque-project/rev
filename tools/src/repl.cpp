#include "core.hpp"
#include "reader.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include <readline/readline.h>
#include <readline/history.h>

#include <stdlib.h>
#include <unistd.h>

void start_repl() {

  if (isatty(0)) {
    read_history(".booti_history");
  }

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
      // this prevents copy/paste gobbledigook from bombing
      // the history
      if (isatty(0)) {
        //auto hist = current_history();
        //if (!hist || strcmp(hist->line, line)) {
        add_history(line);
        write_history(".booti_history");
        //}
      }

      auto result = rev::eval(form);
      pr(result);
    }
    catch(rev::rdr::reader_exception& e) {
      std::cout << "Error while reading object: " << e.what() << std::endl;
    }
    catch(std::exception& e) {
      std::cerr << e.what() << std::endl;
      rev::stack_trace();
#ifdef _DEBUG
      throw;
#endif
    }

    free(line);
  }
}

int main(int argc, char** argv) {

#ifdef _DEBUG
  std::cout << "Starting repl in debug mode" << std::endl;
#endif

  std::string sources = "";
  if (char* s = getenv("REV_SOURCE_PATH")) {
    sources = s;
  }
  // TODO: make source path configurable via parameter too

  rev::boot(1 << 16, sources);

  // load script if provided
  if (argc > 1) {
    std::fstream file(argv[1]);
    if (file.peek() == '#') {
      std::string line;
      std::getline(file, line);
    }
    try {
      rev::load_stream(file);
    }
    catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      rev::stack_trace();
    }
    catch (...) {
      std::cerr << "Unhandled exeption: " << std::endl;
      rev::stack_trace();
    }
  }
  else {
    start_repl();
  }

  return 0;
}
