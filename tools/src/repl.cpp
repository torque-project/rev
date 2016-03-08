#include "core.hpp"
#include "reader.hpp"
#include "adapter.hpp"

#include <iostream>
#include <stdexcept>
#include <sstream>

#include <readline/readline.h>
#include <readline/history.h>

#include <stdlib.h>

int main(int argc, char** argv) {

#ifdef _DEBUG
  std::cout << "Starting repl in debug mode" << std::endl;
#endif

  read_history(".repl_history");

  //try {

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

      //try {
        auto form = rev::read(line);

        // save line in history once we've successfully read it
        // this orevents copy/paste gobbledigook from bombing
        // the history
        add_history(line);
        write_history(".repl_history");

        auto result = rev::eval(form);
        std::cout << str(result)->data() << std::endl;
        /*}
      catch(rev::rdr::reader_exception& e) {
        std::cout << "Error while reading object: " << e.what() << std::endl;
      }
      catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
#ifdef _DEBUG
        throw;
#endif
      }
        */
      free(line);
    }
    /*}
  catch(std::exception& e) {
    std::cerr << "Can't boot VM: " << e.what() << std::endl;
    }*/
}
