#pragma once

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
}
