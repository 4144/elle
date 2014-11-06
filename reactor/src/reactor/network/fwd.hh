#ifndef INFINIT_REACTOR_NETWORK_FWD_HH
# define INFINIT_REACTOR_NETWORK_FWD_HH

# include <reactor/fwd.hh>

namespace reactor
{
  namespace network
  {
    class Buffer;
    template <typename AsioSocket, typename EndPoint>
    class PlainSocket;
    class Server;
    class SSLServer;
    class SSLSocket;
    class Socket;
    class TCPServer;
    class TCPSocket;
    class UDPServer;
    class UDPServerSocket;
    class UDPSocket;

    /// A byte travelling on the network.
    typedef unsigned char Byte;
    /// A number of bytes.
    typedef unsigned int Size;

  }
}

#endif
