#ifndef ELLE_WITH_HXX
# define ELLE_WITH_HXX

# include <utility>

# include <elle/log.hh>
# include <elle/assert.hh>
# include <elle/log.hh>

namespace elle
{
  /*-------------.
  | Construction |
  `-------------*/

  template <typename T>
  template <typename ... Args>
  With<T>::With(Args&&... args):
    _used(false),
    _value(reinterpret_cast<T*>(this->_data))
  {
    ELLE_LOG_COMPONENT("elle.With");
    ELLE_TRACE("%s: construct", *this)
      new (this->_value) T(std::forward<Args>(args)...);
  }

  template <typename T>
  With<T>::With(With<T>&& value):
    _used(value._used),
    _value(reinterpret_cast<T*>(this->_data))
  {
    ELLE_LOG_COMPONENT("elle.With");
    ELLE_TRACE("%s: construct by copy", *this)
      new (this->_value) T(std::move(*this->_value));
  }

  template <typename T>
  With<T>::~With()
  {
    if (!this->_used)
      try
      {
        this->_value->~T();
      }
      catch (...)
      {
        ELLE_ABORT("destructor of unused elle::With threw: %s",
                   elle::exception_string());
      }
  }

  /*--------.
  | Running |
  `--------*/

  template <typename T>
  class
  ReturnHolder
  {
  public:
    template <typename F, typename V>
    inline
    ReturnHolder(F const& f, V& v):
      _value(f(v))
    {}

    inline
    T&&
    value()
    {
      return std::move(this->_value);
    }

  private:
    T _value;
  };

  template <>
  class
  ReturnHolder<void>
  {
  public:
    template <typename F, typename V>
    inline
    ReturnHolder(F const& f, V& v)
    {
      f(v);
    }

    inline
    void
    value()
    {}
  };

  template <typename T>
  template <typename F>
  auto
  With<T>::operator <<(F const& action) -> decltype(action())
  {
    return this->_run([&] (T&) { return action(); });
  }

  template <typename T>
  template <typename F>
  auto
  With<T>::operator <<(F const& action) -> decltype(action(*(T*)(nullptr)))
  {
    return this->_run(action);
  }

  template <typename T>
  template <typename F>
  auto
  With<T>::_run(F const& action) -> decltype(action(*(T*)(nullptr)))
  {
    typedef decltype(action(*(T*)(nullptr))) Value;

    ELLE_LOG_COMPONENT("elle.With");

    ELLE_ASSERT(!this->_used);
    this->_used = true;
    bool succeeded = false;
    try
    {
      ReturnHolder<Value> res(action, *this->_value);
      succeeded = true;
      ELLE_TRACE("%s: destruct", *this)
        this->_value->~T();
      return res.value();
    }
    catch (...)
    {
      ELLE_TRACE("%s: caught exception with succeeded=%s: %s",
                 *this, succeeded, elle::exception_string());
      if (succeeded)
        throw;
      auto e = std::current_exception();
      try
      {
        ELLE_TRACE("%s: destruct", *this)
          this->_value->~T();
      }
      catch (...)
      {
        ELLE_ERR("losing exception: %s", elle::exception_string(e));
        ELLE_ERR("overriden by: %s", elle::exception_string());
        throw;
      }
      ELLE_TRACE("%s: rethrowing exception '%s'",
                 *this, elle::exception_string(e));
      /* Do not use 'throw;' here! ~T might have yielded and current_exception
       * might not be coroutine-safe (observed on gcc 4.8.0 linux)
       */
      std::rethrow_exception(e);
    }
  }
}


#endif
