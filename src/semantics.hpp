#pragma once

#include "momentum/semantics.hpp"

namespace rev {

  /**
   * @ns semantics
   * @brief Defines various semantic helpers like pointer types
   *
   */
  namespace semantics {

    template<typename T>
    struct always { typedef void type; };

    /**
     * select the pointer type of a type by either
     * using T::p or defaulting to T* if no such typedef
     * exists within T
     *
     */
    template<
      typename T,
      typename P = void>
    struct p {
      typedef T* type;
    };

    template<typename T>
    struct p<T, typename always<typename T::p>::type> {
      typedef typename T::p type;
    };
  }

  namespace imu {

    namespace semantics {

      template<>
      struct p<list_t, void> {
        typedef list_t* p;
      };

      template<>
      struct p<vector_t, void> {
        typedef vector_t* p;
      };

      template<>
      struct p<map_t, void> {
        typedef map_T_t* p;
      };
    }
  }
}
