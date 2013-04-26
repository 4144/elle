#define BOOST_TEST_MODULE udt-socket
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <reactor/network/udt-socket.hh>
#include <reactor/network/udt-server.hh>
#include <reactor/network/udp-socket.hh>
#include <reactor/network/udp-server-socket.hh>
#include <elle/container/vector.hh>
#include <reactor/scheduler.hh>
#include <reactor/network/buffer.hh>
#include <elle/memory.hh>

using namespace reactor::network;
using namespace reactor;

// This test case test the UDTServer construction with an UDPSocket
BOOST_AUTO_TEST_CASE(udtsocket)
{
  Scheduler sched;

  auto test_fn = [&]
  {
    auto sock = elle::make_unique<UDPSocket>(sched, "infinit.im", 9999);
    std::vector<char> buff(512);

    std::string msg{"unit_testing"}; // Will do an error, but it's normal
    sock->write(Buffer(msg.data(), msg.size()));
    std::cout << sock->local_endpoint() << std::endl;
    UDTServer udtsocket(sched, std::move(sock));
  };
  Thread t(sched, "test", test_fn);
  sched.run();
}
