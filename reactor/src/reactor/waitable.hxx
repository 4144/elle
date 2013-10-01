#ifndef INFINIT_REACTOR_WAITABLE_HXX
# define INFINIT_REACTOR_WAITABLE_HXX

#include <exception>

namespace reactor
{
  template <typename Exception, typename... Args>
  void
  Waitable::_raise(Args&&... args)
  {
    try
    {
      throw Exception(std::forward<Args>(args)...);
    }
    catch (...)
    {
      this->_raise(std::current_exception());
    }
  }
}


#endif
