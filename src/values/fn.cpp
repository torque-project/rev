#include "../values.hpp"

namespace rev {
  template<>
  type_t value_base_t<fn_t>::prototype("Fn.0");

  template<>
  type_t* prototype<fn_t>() {
    return & value_base_t<fn_t>::prototype;
  }
}
