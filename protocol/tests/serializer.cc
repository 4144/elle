#include <protocol/exceptions.hh>
#include <protocol/Serializer.hh>

#include <reactor/asio.hh>
#include <reactor/Barrier.hh>
#include <reactor/Scope.hh>
#include <reactor/network/buffer.hh>
#include <reactor/network/exception.hh>
#include <reactor/network/tcp-server.hh>
#include <reactor/scheduler.hh>

#include <elle/test.hh>
#include <elle/With.hh>

ELLE_LOG_COMPONENT("infinit.protocol.test");

class SocketInstrumentation
{
public:
  SocketInstrumentation():
    _router(*reactor::Scheduler::scheduler(), "router",
            std::bind(&SocketInstrumentation::_route, this)),
    _a_conf(),
    _b_conf()
  {
    this->_a_server.listen();
    this->_b_server.listen();
    this->_alice.reset(new reactor::network::TCPSocket("127.0.0.1",
                                                       this->_a_server.port()));
    this->_bob.reset(new reactor::network::TCPSocket("127.0.0.1",
                                                     this->_b_server.port()));
    reactor::yield();
    reactor::yield();
  }

  ~SocketInstrumentation()
  {
    this->_router.terminate_now();
  }

  reactor::network::TCPSocket&
  alice()
  {
    return *this->_alice;
  }

  reactor::network::TCPSocket&
  bob()
  {
    return *this->_bob;
  }

  void
  alice_quota(int quota)
  {
    this->_a_conf.quota = quota;
  }

  void
  bob_quota(int quota)
  {
    this->_b_conf.quota = quota;
  }

  void
  alice_corrupt(int offset, char mask = 0xFF)
  {
    this->_a_conf.corrupt_offset = offset;
    this->_a_conf.corrupt_mask = mask;
  }

private:
  void
  _route()
  {
    using reactor::network::Buffer;

    std::unique_ptr<reactor::network::TCPSocket> a(this->_a_server.accept());
    std::unique_ptr<reactor::network::TCPSocket> b(this->_b_server.accept());

    elle::With<reactor::Scope>() << [&](reactor::Scope& scope)
    {
      auto route = [&] (reactor::network::TCPSocket* a,
                        reactor::network::TCPSocket* b,
                        Conf& conf)
        {
          try
          {
            int routed = 0;
            bool relay = true;
            while (relay)
            {
              char buffer[1024];
              int size = a->read_some(Buffer(buffer, sizeof(buffer)));
              if (conf.corrupt_offset >= 0 && \
                  conf.corrupt_offset >= routed && \
                  conf.corrupt_offset < routed + size)
              {
                int offset = conf.corrupt_offset - routed;
                ELLE_LOG("%s: corrupt byte %s", *this, offset);
                buffer[offset] ^= conf.corrupt_mask;
              }
              routed += size;
              if (conf.quota >= 0 && routed > conf.quota)
              {
                ELLE_LOG("%s: quota reached, interrupt", *this);
                size = size - (routed - conf.quota);
                relay = false;
                routed = conf.quota;
              }
              ELLE_TRACE("%s: route %s bytes", *this, size);
              b->write(elle::ConstWeakBuffer(buffer, size));
            }
          }
          catch (reactor::network::ConnectionClosed const&)
          {}
          a->close();
          b->close();
        };
      scope.run_background("A to B",
                           std::bind(route, a.get(), b.get(),
                                     std::ref(this->_a_conf)));
      scope.run_background("B to A",
                           std::bind(route, b.get(), a.get(),
                                     std::ref(this->_b_conf)));
      scope.wait();
    };

  }

  reactor::network::TCPServer _a_server;
  reactor::network::TCPServer _b_server;
  std::unique_ptr<reactor::network::TCPSocket> _alice;
  std::unique_ptr<reactor::network::TCPSocket> _bob;
  reactor::Thread _router;

  struct Conf
  {
    Conf():
      quota(-1),
      corrupt_offset(-1),
      corrupt_mask(0)
    {}

    int quota;
    int corrupt_offset;
    char corrupt_mask;
  };
  Conf _a_conf;
  Conf _b_conf;
};

static
void
dialog(std::function<void (SocketInstrumentation&)> const& conf,
       std::function<void (infinit::protocol::Serializer&)> const& a,
       std::function<void (infinit::protocol::Serializer&)> const& b)
{
  reactor::Scheduler sched;

  reactor::Thread main(
    sched, "main",
    [&] ()
    {
      SocketInstrumentation sockets;
      infinit::protocol::Serializer alice(sockets.alice());
      infinit::protocol::Serializer bob(sockets.bob());
      conf(sockets);

      elle::With<reactor::Scope>() << [&](reactor::Scope& scope)
      {
        scope.run_background("alice", std::bind(a, std::ref(alice)));
        scope.run_background("bob", std::bind(b, std::ref(bob)));
        scope.wait();
      };
    });

  sched.run();
}

static
void
exchange_packets()
{
  dialog([] (SocketInstrumentation&) {},
         [] (infinit::protocol::Serializer& s)
         {
           {
             infinit::protocol::Packet p;
             p << "some data 42";
             s.write(p);
           }
           {
             infinit::protocol::Packet p = s.read();
             std::string res;
             p >> res;
             BOOST_CHECK_EQUAL(res, "goody");
             BOOST_CHECK(!(p >> res));
           }
         },
         [] (infinit::protocol::Serializer& s)
         {
           {
             infinit::protocol::Packet p = s.read();
             std::string some, data;
             int i;
             p >> some;
             BOOST_CHECK_EQUAL(some, "some");
             p >> data;
             BOOST_CHECK_EQUAL(data, "data");
             p >> i;
             BOOST_CHECK_EQUAL(i, 42);
             BOOST_CHECK(!(p >> data));
           }
           {
             infinit::protocol::Packet p;
             p << "goody";
             s.write(p);
           }
         });
}

static
void
connection_lost_reader()
{
  dialog(
    [] (SocketInstrumentation& sockets)
    {
      sockets.alice_quota(96);
    },
    [] (infinit::protocol::Serializer& s)
    {
      infinit::protocol::Packet p;
      char buffer[1025];
      memset(buffer, 0xAA, sizeof(buffer));
      buffer[sizeof(buffer) - 1] = 0;
      p << buffer;
      s.write(p);
    },
    [] (infinit::protocol::Serializer& s)
    {
      BOOST_CHECK_THROW(s.read(), reactor::network::ConnectionClosed);
    });
}

static
void
connection_lost_sender()
{
  dialog(
    [] (SocketInstrumentation& sockets)
    {
      sockets.alice_quota(4);
    },
    [] (infinit::protocol::Serializer& s)
    {
      infinit::protocol::Packet p;
      p << "data";
      s.write(p);
      // Getting an error from TCP writes is a bit touchy.
      try
      {
        while (true)
        {
          reactor::sleep(100_ms);
          s.write(p);
        }
      }
      catch (reactor::network::ConnectionClosed const&)
      {
        return;
      }
      BOOST_FAIL("ConnectionClosed exception was expected");
    },
    [] (infinit::protocol::Serializer&)
    {});
}

static
void
corruption()
{
  dialog(
    [] (SocketInstrumentation& sockets)
    {
      sockets.alice_corrupt(1024);
    },
    [] (infinit::protocol::Serializer& s)
    {
      infinit::protocol::Packet p;
      char buffer[1025];
      memset(buffer, 0xAA, sizeof(buffer));
      buffer[sizeof(buffer) - 1] = 0;
      p << buffer;
      s.write(p);
    },
    [] (infinit::protocol::Serializer& s)
    {
      BOOST_CHECK_THROW(s.read(), infinit::protocol::ChecksumError);
    });
}

ELLE_TEST_SUITE()
{
  auto& suite = boost::unit_test::framework::master_test_suite();
  suite.add(BOOST_TEST_CASE(exchange_packets), 0, 3);
  suite.add(BOOST_TEST_CASE(connection_lost_reader), 0, 3);
  suite.add(BOOST_TEST_CASE(connection_lost_sender), 0, 3);
  suite.add(BOOST_TEST_CASE(corruption), 0, 3);
}
