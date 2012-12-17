#ifndef INFINIT_REACTOR_NETWORK_SOCKET_HH
# define INFINIT_REACTOR_NETWORK_SOCKET_HH

# include <elle/IOStream.hh>
# include <elle/network/Locus.hh>
# include <elle/idiom/Close.hh>

# include <reactor/duration.hh>
# include <reactor/network/fwd.hh>

namespace reactor
{
  namespace network
  {
    template <typename AsioSocket>
    class SocketOperation;

    class Socket: public elle::IOStream
    {
      /*---------.
      | Typedefs |
      `---------*/
      public:
        /// Self type.
        typedef Socket Self;

      /*-------------.
      | Construction |
      `-------------*/
      public:
        /// Create an unbound socket.
        Socket(Scheduler& sched);
        /// Destroy a socket.
        virtual ~Socket();

      /*------.
      | Write |
      `------*/
      public:
        virtual void write(Buffer buffer) = 0;
        void write(const char* data);

      /*-----.
      | Read |
      `-----*/
      public:
        virtual void read(Buffer buffer,
                          DurationOpt timeout = DurationOpt());
        virtual Size read_some(Buffer buffer,
                               DurationOpt timeout = DurationOpt()) = 0;

      /*-----------.
      | Scheduling |
      `-----------*/
      public:
        Scheduler& scheduler();
      private:
        Scheduler& _sched;

     /*------.
     | Locus |
     `------*/
      public:
        virtual elle::network::Locus local_locus() const = 0;
        virtual elle::network::Locus remote_locus() const = 0;

     /*----------------.
     | Pretty printing |
     `----------------*/
      public:
        virtual void print(std::ostream& s) const = 0;
    };
    std::ostream& operator << (std::ostream& s, const Socket& socket);

    template <typename AsioSocket_>
    class PlainSocket: public Socket
    {
      /*---------.
      | Typedefs |
      `---------*/
      public:
        /// Self type.
        typedef Socket Self;
        /// Super type.
        typedef Socket Super;
        /// Underlying asio socket type.
        typedef AsioSocket_ AsioSocket;
        /// End point type for the asio socket type.
        typedef typename AsioSocket::endpoint_type EndPoint;

    /*-------------.
    | Construction |
    `-------------*/
    protected:
      /// Create and connect socket.
      PlainSocket(Scheduler& sched, const EndPoint& peer, DurationOpt timeout = DurationOpt());
      /// Create wrapping socket.
      PlainSocket(Scheduler& sched, AsioSocket* socket);
      /// Destroy a socket.
      virtual ~PlainSocket();

    /*-----------.
    | Connection |
    `-----------*/
    private:
      void _connect(const EndPoint& peer, DurationOpt timeout = DurationOpt());
      void _disconnect();

    /*-----------.
    | Properties |
    `-----------*/
    public:
      virtual elle::network::Locus local_locus() const;
      virtual elle::network::Locus remote_locus() const;
      EndPoint peer() const;
      EndPoint local_endpoint() const;

    /*----------------.
    | Pretty printing |
    `----------------*/
    public:
      virtual void print(std::ostream& s) const;

    /*------------.
    | Asio socket |
    `------------*/
    protected:
      friend class TCPServer;
      friend class TCPSocket;
      friend class UDPServer;
      friend class UDPSocket;
      template <typename AsioSocket>
      friend class SocketOperation;
      AsioSocket* _socket;
      EndPoint _peer;
    };
  }
}

#endif
