#pragma once

#include "values.hpp"

#include <memory>

namespace rev {

  /**
   * This offers a seqs defined in user code. This
   * is used to allow runtime macros to return arbitrary
   * values as long as they implement the seq interface.
   *
   */
  struct rt_seq_t : public imu::no_mixin {

    typedef std::shared_ptr<rt_seq_t> p;

    value_t::p _head;

    inline rt_seq_t(const value_t::p& head)
      : _head(head)
    {}

    inline bool is_empty() const {
      return !_head;
    }

    inline value_t::p first() const {
      return  protocol_t::dispatch_(protocol_t::seq, 0, _head);
    }

    inline p rest() const {
      // use next instead of rest, since next returns nil at
      // the ned of a seq
      return imu::nu<rt_seq_t>(
        protocol_t::dispatch_(protocol_t::next, 0, _head));
    }

    static p seq(const value_t::p& v) {
      auto s = protocol_t::dispatch_(protocol_t::seqable, 0, v);
      return imu::nu<rt_seq_t>(s);
    }
  };

  template<typename T>
  inline typename T::p nativize(const value_t::p& v) {
    if (auto x = as_nt<T>(v)) { return x; }
    auto s = rt_seq_t::seq(v);
    return into(imu::nu<T>(), s);
  }

  struct rt_vec_t : public imu::no_mixin {

    typedef std::shared_ptr<rt_vec_t> p;

    value_t::p _vec;

    inline rt_vec_t(const value_t::p& v)
      : _vec(v)
    {}

    inline value_t::p nth(uint64_t n) const {
      int_t i(n);
      return protocol_t::dispatch_(protocol_t::indexed, 0, _vec, &i);
    }

    inline value_t::p key() const {
      return nth(0);
    }

    inline value_t::p val() const {
      return nth(1);
    }
  };
}
