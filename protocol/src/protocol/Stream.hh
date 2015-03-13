#ifndef INFINIT_PROTOCOL_STREAM_HH
# define INFINIT_PROTOCOL_STREAM_HH

# include <iosfwd>

# include <elle/Printable.hh>

# include <reactor/fwd.hh>

# include <protocol/Packet.hh>

namespace infinit
{
  namespace protocol
  {
    class Stream: public elle::Printable
    {
    /*-------------.
    | Construction |
    `-------------*/
    public:
      Stream(reactor::Scheduler& scheduler);
      virtual ~Stream();

    /*-----------.
    | Properties |
    `-----------*/
    public:
      reactor::Scheduler& scheduler();
    private:
      reactor::Scheduler& _scheduler;

    /*----------.
    | Receiving |
    `----------*/
    public:
      virtual Packet read() = 0;

    /*--------.
    | Sending |
    `--------*/
    public:
      void
      write(Packet& packet);
    protected:
      virtual
      void
      _write(Packet& packet) = 0;

    /*------------------.
    | Int serialization |
    `------------------*/
    protected:
      void _uint32_put(std::ostream& s, uint32_t  i);
      uint32_t _uint32_get(std::istream& s);
    };
  }
}

#endif
