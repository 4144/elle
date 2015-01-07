#include <elle/assert.hh>
#include <elle/Backtrace.hh>
#include <elle/log.hh>
#include <elle/os/environ.hh>

#include <iostream>
#include <sstream>

ELLE_LOG_COMPONENT("elle.assert");

namespace elle
{
  void
  _elle_assert(bool predicate,
          std::string const& message,
          char const* file,
          int line)
  {
    if (!predicate)
      _abort(message, file, line);
  }

  void
  unreachable()
  {
    _abort("unreachable code reached", "", 0);
  }

  void
  _abort(std::string const& message,
         char const* file,
         int line)
  {
    if (std::current_exception() != std::exception_ptr{})
    {
      // There is already an exception happening.
      auto e = std::current_exception();
      ELLE_ERR("raising an assert (%s at %s:%s) with an exception already in flight: %s",
               message, file, line, elle::exception_string());
    }

    if (elle::os::inenv("ELLE_REAL_ASSERT"))
    {
      ELLE_ERR("%s: (%s:%s)", message.c_str(), file, line);
      std::abort();
    }

    throw elle::AssertError(message.c_str(), file, line);
  }

  AssertError::AssertError(char const* condition,
                           char const* file,
                           Size line) throw()
  {
    try
    {
      this->_what = elle::sprintf("assertion '%s' failed at %s:%s",
                                  condition, file, line);
      ELLE_ERR("%s", this->_what)
        ELLE_ERR("%s", Backtrace::current());
    }
    catch (...)
    {
      ELLE_WARN("could not build the error string for %s at %s:%s",
                condition,
                file,
                line);
    }
  }

  const char*
  AssertError::what() const throw()
  {
    if (_what.size())
      return _what.c_str();
    else
      return "AssertError (could not build the error string)";
  }

}
