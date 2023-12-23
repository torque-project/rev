#include "../values.hpp"

namespace rev {

  template<>
  type_t value_base_t<array_t>::prototype("Array.0");

  template<>
  type_t* prototype<array_t>() {
    return & value_base_t<array_t>::prototype;
  }
}
