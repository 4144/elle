#include <elle/log.hh>

#include <reactor/network/exception.hh>
#include <reactor/network/resolve.hh>
#include <reactor/operation.hh>
#include <reactor/scheduler.hh>

ELLE_LOG_COMPONENT("reactor.network.resolve");

namespace reactor
{
  namespace network
  {
    template <typename Resolver>
    class Resolution: public Operation
    {
    public:
      typedef typename Resolver::endpoint_type EndPoint;
      Resolution(const std::string& hostname, const std::string& service):
        Operation(*reactor::Scheduler::scheduler()),
        _resolver(reactor::Scheduler::scheduler()->io_service()),
        _canceled(false),
        _hostname(hostname),
        _service(service),
        _end_point()
      {}

      virtual
      void
      print(std::ostream& stream) const override
      {
        stream << "resolution of " << this->_hostname << ":" << this->_service;
      }

      virtual
      const
      char*
      type_name() const
      {
        static const char* name = "name resolution";
        return name;
      }

    protected:
      virtual
      void
      _abort()
      {
        ELLE_TRACE_SCOPE("%s: abort", *this);
        this->_canceled = true;
        this->_resolver.cancel();
        reactor::wait(*this);
      }

      virtual
      void
      _start()
      {
        ELLE_TRACE("resolve %s:%s", this->_hostname, this->_service);
        typename Resolver::query query(this->_hostname, this->_service);
        this->_resolver.async_resolve(
          query,
          boost::bind(&Resolution::_wakeup, this, _1, _2));
      }

    private:
      void
      _wakeup(const boost::system::error_code& error,
              typename Resolver::iterator it)

      {
        if (this->_canceled)
          ELLE_TRACE_SCOPE("%s: canceled", *this);
        else if (error)
        {
          ELLE_TRACE_SCOPE("%s: ended with error: %s", *this, error.message());
          this->_raise<ResolutionError>(this->_hostname, error.message());
        }
        else if (it == typename Resolver::iterator::basic_resolver_iterator())
        {
          // From the boost documentation:
          //   A successful resolve operation is guaranteed to pass at least one
          //   entry to the handler.
          // This assumption is false on wine.
          ELLE_TRACE_SCOPE(
            "%s: ended with no error but an empty address list", *this);
          this->_raise<ResolutionError>(
            this->_hostname, "host not found: address list is empty");
        }
        else
        {
          ELLE_TRACE_SCOPE("%s: ended", *this);
          this->_end_point = *it;
        }
        this->done();
      }

      ELLE_ATTRIBUTE(Resolver, resolver);
      ELLE_ATTRIBUTE(bool, canceled);
      ELLE_ATTRIBUTE(std::string, hostname);
      ELLE_ATTRIBUTE(std::string, service);
      ELLE_ATTRIBUTE_R(EndPoint, end_point);
    };

    template <typename Resolver>
    typename Resolver::endpoint_type
    resolve(const std::string& hostname, const std::string& service)
    {
      Resolution<Resolver> resolution(hostname, service);
      resolution.run();
      return resolution.end_point();
    }

    boost::asio::ip::tcp::endpoint
    resolve_tcp(const std::string& hostname, const std::string& service)
    {
      return resolve<boost::asio::ip::tcp::resolver>(hostname, service);
    }

    boost::asio::ip::udp::endpoint
    resolve_udp(const std::string& hostname, const std::string& service)
    {
      return resolve<boost::asio::ip::udp::resolver>(hostname, service);
    }
  }
}
