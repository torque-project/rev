#pragma once

#include "values.hpp"

#include <cassert>
#include <stack>
#include <vector>

namespace rev {

  struct ctx_t {

    enum class scope_t {
      local,
      env,
      global
    };

    struct lookup_t {

      scope_t    _scope;
      value_t::p _var;

      inline operator bool() const {
        return _var;
      }

      inline value_t::p operator* () const {
        return _var;
      }

      inline value_t::p operator-> () const {
        return _var;
      }

      inline bool is_local() const {
        return _scope == scope_t::local;
      }

      inline bool is_global() const {
        return _scope == scope_t::global;
      }
    };

    struct state_t {

      typedef std::shared_ptr<state_t> p;

      uint64_t stack_space;

      inline state_t() : stack_space(0) {}
    };

    map_t::p   _env;
    map_t::p   _locals;
    map_t::p   _closed_overs;

    state_t::p _state;

    int64_t    _recur_point;
    list_t::p  _recur_syms;

    inline ctx_t()
      : _env(imu::nu<map_t>())
      , _locals(imu::nu<map_t>())
      , _state(state_t::p(new state_t()))
    {}

    inline ctx_t(
      const ctx_t&      parent,
      const map_t::p&   e,
      const map_t::p&   l,
      const map_t::p&   c,
      const state_t::p& s)
      : _env(e)
      , _locals(l)
      , _closed_overs(c)
      , _state(s)
      , _recur_point(parent._recur_point)
      , _recur_syms(parent._recur_syms)
    {}

    inline ctx_t(
      const ctx_t& parent,
      int64_t rp,
      const list_t::p& rs)
      : _env(parent._env)
      , _locals(parent._locals)
      , _closed_overs(parent._closed_overs)
      , _state(parent._state)
      , _recur_point(rp)
      , _recur_syms(rs)
    {}

    inline ctx_t closure() const {
      return ctx_t(
        *this,
        imu::merge(_env, _locals),
        imu::nu<map_t>(),
        imu::nu<map_t>(),
        state_t::p(new state_t()));
    }

    inline ctx_t body() {
      return ctx_t(
        *this,
        _env,
        _locals,
        _closed_overs,
        state_t::p(new state_t()));
    }

    inline ctx_t local(const sym_t::p& sym) {
      return ctx_t(
        *this,
        _env,
        imu::assoc(_locals, sym, imu::nu<int_t>(alloca())),
        _closed_overs,
        _state);
    }

    inline ctx_t recur(int64_t rp, const list_t::p& rs) const {
      return ctx_t(*this, rp, rs);
    }

    inline map_t::p local() const {
      return _locals;
    }

    inline map_t::p env() const {
      return _env;
    }

    inline decltype(auto) closed_overs() const {
      return imu::keys(_closed_overs);
    }

    inline uint64_t alloca() {
      return _state->stack_space++;
    }

    inline uint64_t stack_space() const {
      return _state->stack_space;
    }

    inline int64_t recur_to() const {
      return _recur_point;
    }

    inline list_t::p recur_syms() const {
      return _recur_syms;
    }

    inline int64_t close_over(const sym_t::p& sym) {
      assert(_closed_overs && "Can't close over in global context");
      return _closed_overs->assoc(sym, sym);
    }

    inline maybe<value_t::p> operator[](const sym_t::p& sym) const {
      return imu::get(_locals, sym);
    }
  };

  typedef std::vector<int64_t> thread_t;
  typedef int64_t*             stack_t;

  void compile(const value_t::p& form, ctx_t& ctx);
  void compile(const value_t::p& form, ctx_t& ctx, thread_t& t);

  template<typename T>
  uint32_t compile_all(const T& forms, ctx_t& ctx, thread_t& t) {
    return imu::reduce([&](uint32_t n, const value_t::p& form) {
        compile(form, ctx, t);
        return n + 1;
      },
      0,
      forms);
  }

  ctx_t::lookup_t resolve(ctx_t& ctx, const sym_t::p& sym);

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

  /**
   * Returns the length of the function pointed to by address,
   * where length is defined by the distance to the closest return
   * statement (there is only one per function).
   *
   */
  int64_t compute_fn_length(int64_t address);
}
