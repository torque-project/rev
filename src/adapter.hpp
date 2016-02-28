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
      void* args[] = {(void*) _head};
      return  protocol_t::dispatch(protocol_t::seq, 0, args, 1);
    }

    inline p rest() const {
      void* args[] = {(void*) _head};
      // use next instead of rest, since next returns nil at
      // the ned of a seq
      return imu::nu<rt_seq_t>(
        protocol_t::dispatch(protocol_t::next, 0, args, 1));
    }
  };
}
