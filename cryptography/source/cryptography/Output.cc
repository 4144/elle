#include <cryptography/Output.hh>

namespace infinit
{
  namespace cryptography
  {
    /*-------------.
    | Construction |
    `-------------*/

    Output::Output(elle::Natural64 const size):
      _buffer(size)
    {
    }

    Output::Output(Output const& other):
      _buffer(other._buffer.contents(), other._buffer.size())
    {
    }

    Output::Output(Output&& other):
      _buffer(std::move(other._buffer))
    {
    }

    /*----------.
    | Operators |
    `----------*/

    elle::Boolean
    Output::operator ==(Output const& other) const
    {
      if (this == &other)
        return (true);

      return (this->_buffer == other._buffer);
    }

    elle::Boolean
    Output::operator <(Output const& other) const
    {
      if (this == &other)
        return (false);

      return (this->_buffer < other._buffer);
    }

    elle::Boolean
    Output::operator <=(Output const& other) const
    {
      if (this == &other)
        return (true);

      return (this->_buffer <= other._buffer);
    }

    elle::Boolean
    Output::operator ==(Input const& other) const
    {
      return (elle::WeakBuffer(this->_buffer) == other.buffer());
    }

    elle::Boolean
    Output::operator <(Input const& other) const
    {
      return (elle::WeakBuffer(this->_buffer) < other.buffer());
    }

    elle::Boolean
    Output::operator <=(Input const& other) const
    {
      return (elle::WeakBuffer(this->_buffer) <= other.buffer());
    }

    /*---------.
    | Dumpable |
    `---------*/

    elle::Status
    Output::Dump(elle::Natural32           margin) const
    {
      elle::String      alignment(margin, ' ');

      std::cout << alignment << "[Output] " << this << std::endl;

      this->_buffer.dump(margin + 2);

      return elle::Status::Ok;
    }

    /*----------.
    | Printable |
    `----------*/

    void
    Output::print(std::ostream& stream) const
    {
      stream << this->_buffer;
    }
  }
}
