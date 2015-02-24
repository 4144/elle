#if defined(ELLE_CRYPTOGRAPHY_ROTATION)

# include <cryptography/rsa/Seed.hh>
# include <cryptography/Exception.hh>
# include <cryptography/PublicKey.hh>
# include <cryptography/PrivateKey.hh>
# include <cryptography/random.hh>
# include <cryptography/bn.hh>
# include <cryptography/rsa/PublicKey.hh>
# include <cryptography/rsa/PrivateKey.hh>

# include <elle/log.hh>

ELLE_LOG_COMPONENT("infinit.cryptography.rsa.Seed");

//
// ---------- Class -----------------------------------------------------------
//

namespace infinit
{
  namespace cryptography
  {
    namespace rsa
    {
      /*-------------.
      | Construction |
      `-------------*/

      Seed::Seed()
      {
      }

      Seed::Seed(elle::Buffer const& buffer,
                 ::BIGNUM* n):
        _buffer(buffer.contents(), buffer.size()),
        _n(::BN_dup(n))
      {
      }

      Seed::Seed(elle::Buffer&& buffer,
                 ::BIGNUM* n):
        _buffer(std::move(buffer)),
        _n(n)
      {
      }

      Seed::Seed(Seed const& other):
        _buffer(other._buffer.contents(), other._buffer.size()),
        _n(::BN_dup(other._n.get()))
      {
      }

      Seed::Seed(Seed&& other):
        _buffer(std::move(other._buffer)),
        _n(std::move(other._n.get()))
      {
        ELLE_ASSERT_EQ(other._n, nullptr);
      }

      ELLE_SERIALIZE_CONSTRUCT_DEFINE(Seed)
      {
      }

      /*----------.
      | Operators |
      `----------*/

      elle::Boolean
      Seed::operator ==(Seed const& other) const
      {
        if (this == &other)
          return (true);

        ELLE_ASSERT_NEQ(this->_n, nullptr);
        ELLE_ASSERT_NEQ(other._n, nullptr);

        // Compare the buffer and modulus.
        if ((this->_buffer != other._buffer) ||
            (::BN_cmp(this->_n.get(),
                      other._n.get()) != 0))
          return (false);

        return (true);
      }

      /*-----.
      | Seed |
      `-----*/

      elle::Boolean
      Seed::operator ==(cryptography::seed::Interface const& other) const
      {
        if (this == &other)
          return (true);

        if (this->cryptosystem() != other.cryptosystem())
          return (false);

        return (*this == static_cast<Seed const&>(other));
      }

      cryptography::seed::Interface*
      Seed::clone() const
      {
        return (new Seed(*this));
      }

      Cryptosystem
      Seed::cryptosystem() const
      {
        return (Cryptosystem::rsa);
      }

      /*----------.
      | Printable |
      `----------*/

      void
      Seed::print(std::ostream& stream) const
      {
        ELLE_ASSERT_NEQ(this->_n, nullptr);

        stream << "("
               << this->_buffer
               << ", "
               << *this->_n
               << ")";
      }
    }
  }
}

//
// ---------- Generator -------------------------------------------------------
//

namespace infinit
{
  namespace cryptography
  {
    namespace rsa
    {
      namespace seed
      {
        Seed
        generate(cryptography::publickey::Interface const& K,
                 cryptography::privatekey::Interface const& k)
        {
          ELLE_TRACE_FUNCTION(K, k);

          ELLE_ASSERT_EQ(K.cryptosystem(), k.cryptosystem());
          ELLE_ASSERT_EQ(K.cryptosystem(), Cryptosystem::rsa);

          // Cast the interface into an actual RSA public key.
          ELLE_ASSERT_NEQ(dynamic_cast<PublicKey const*>(&K), nullptr);
          PublicKey const& _K = static_cast<PublicKey const&>(K);

          // Return an RSA seed constructed by generating a random buffer
          // or the size of the modulus.
          ELLE_ASSERT_NEQ(_K.key(), nullptr);
          ELLE_ASSERT_NEQ(_K.key()->pkey.rsa, nullptr);
          ELLE_ASSERT_NEQ(_K.key()->pkey.rsa->n, nullptr);

          elle::Buffer buffer =
            random::generate<elle::Buffer>(_K.size());
          ::BIGNUM* n = ::BN_dup(_K.key()->pkey.rsa->n);

          return (Seed(std::move(buffer), n));
        }
      }
    }
  }
}

#endif
