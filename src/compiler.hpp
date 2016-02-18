#pragma once

#include "values.hpp"

#include <stack>
#include <vector>

namespace rev {

  struct ctx_t {

    typedef typename map_t::key_seq::p key_seq_t;

    map_t::p  _env;

    int64_t   _recur_point;
    key_seq_t _recur_syms;

    inline ctx_t()
      : _env(imu::nu<map_t>())
    {}

    inline ctx_t(const map_t::p& e)
      : _env(e)
    {}

    inline ctx_t(const ctx_t& cpy, const map_t::p& e)
      : ctx_t(cpy, e, cpy._recur_point, cpy._recur_syms)
    {}

    inline ctx_t(
      const ctx_t& cpy,
      const map_t::p& e,
      int64_t rp)
      : _env(imu::merge(cpy._env, e)),
        _recur_point(rp),
        _recur_syms(imu::keys(e))
    {}

    inline ctx_t(
      const ctx_t& cpy,
      const map_t::p& e,
      int64_t rp,
      const key_seq_t& rs)
      : _env(imu::merge(cpy._env, e)),
        _recur_point(rp),
        _recur_syms(rs)
    {}

    inline map_t::p env() const {
      return _env;
    }

    inline int64_t recur_to() const {
      return _recur_point;
    }

    inline key_seq_t recur_syms() const {
      return _recur_syms;
    }

    inline maybe<value_t::p> operator[](const sym_t::p& sym) const {
      return imu::get(_env, sym);
    }
  };

  typedef std::vector<int64_t> thread_t;
  typedef std::vector<int64_t> stack_t;

  void compile(const value_t::p& form, const ctx_t& ctx);
  void compile(const value_t::p& form, const ctx_t& ctx, thread_t& t);
  void compile_all(const list_t::p& form, const ctx_t& env);
  void compile_all(const list_t::p& form, const ctx_t& env, thread_t& t);

  thread_t& main_thread();

  /**
   * Commits a thread to the main code area. This is used to build
   * function coee before writing it to the final code area. This allows
   * compiling nested functions without more complex parser code, at the
   * cost of a few additional memcpy calls
   *
   * Return an offset pointing directly before the newly added piece of
   * code
   *
   */
  int64_t finalize_thread(thread_t& t);
}
