#ifndef REACTOR_HTTP_EXCEPTIONS_HH
# define REACTOR_HTTP_EXCEPTIONS_HH

# include <elle/Exception.hh>

# include <reactor/duration.hh>

namespace reactor
{
  namespace http
  {
    /// Fatal error on a request that was not successfully completed.
    class RequestError:
      public elle::Exception
    {
    public:
      typedef elle::Exception Super;
      RequestError(std::string const& url,
                   std::string const& error);
    private:
      ELLE_ATTRIBUTE_R(std::string, url);
      ELLE_ATTRIBUTE_R(std::string, error);
    };

    /// The server didn't send any response.
    class EmptyResponse:
      public RequestError
    {
    public:
      typedef RequestError Super;
      EmptyResponse(std::string const& url);
    };

    /// The request timed out.
    class Timeout:
      public RequestError
    {
    public:
      typedef RequestError Super;
      Timeout(std::string const& url,
              reactor::Duration const& timeout);
      ELLE_ATTRIBUTE_R(reactor::Duration, timeout);
    };
  }
}

#endif
