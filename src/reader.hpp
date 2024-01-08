#pragma once

#include "values.hpp"

#include <exception>
#include <functional>
#include <string>

namespace rev {

  namespace rdr {

    struct reader_exception : public std::exception {};

    /**
     * Gets thrown when the reader encounters the end of
     * the input stream while still reading a form
     *
     */
    struct eos : public reader_exception {
      std::string msg;

      eos(const std::string &form) 
        : msg("The reader reached the end of input "  \
        "while reading an unfinished object: " + form) 
      {}

      const char* what() const noexcept {
        return msg.c_str();
      }
    };

    /**
     * Gets thrown when the reader encounters an unbalanced
     * form in its input
     *
     */
    struct unbalanced : public reader_exception {

      std::string msg;

      inline unbalanced(char t)
        : msg(std::string("Unbalanced paranthesis: ") + t)
      {}

      const char* what() const noexcept {
        return msg.c_str();
      }
    };

    struct escape_error : public reader_exception {
      const char* what() const noexcept {
        return "Incomplete or unknown escape sequence";
      }
    };

    /**
     * Read the first form from a string
     *
     */
    value_t::p read(const std::string&);

    /**
     * Read the first form from an input stream
     *
     */
    value_t::p read(std::istream&);
  }
}
