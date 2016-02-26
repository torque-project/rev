#include "../values.hpp"

namespace rev {

  uint64_t protocol_t::id = 0;

  template<>
  type_t value_base_t<protocol_t>::prototype("Protocol");
}
