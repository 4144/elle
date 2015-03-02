#if defined(ELLE_CRYPTOGRAPHY_ROTATION)

# ifndef INFINIT_CRYPTOGRAPHY_RSA_SEED_HH
#  define INFINIT_CRYPTOGRAPHY_RSA_SEED_HH

#  include <cryptography/fwd.hh>
#  include <cryptography/types.hh>
#  include <cryptography/Seed.hh>

#  include <elle/types.hh>
#  include <elle/attribute.hh>
#  include <elle/operator.hh>
#  include <elle/Buffer.hh>
#  include <elle/Printable.hh>
#  include <elle/serialize/fwd.hh>
#  include <elle/serialize/DynamicFormat.hh>
#  include <elle/serialize/construct.hh>
#  include <elle/concept/Uniquable.hh>

//
// ---------- Class -----------------------------------------------------------
//

namespace infinit
{
  namespace cryptography
  {
    namespace rsa
    {
      /// Represent an RSA seed which can be used to deterministically generate
      /// RSA key pairs, private or public keys given a buffer of random data.
      class Seed:
        public cryptography::seed::Interface,
        public elle::serialize::SerializableMixin<Seed>,
        public elle::serialize::DynamicFormat<Seed>,
        public elle::concept::MakeUniquable<Seed>
      {
        /*-------------.
        | Construction |
        `-------------*/
      public:
        Seed(); // XXX[to deserialize]
        /// Construct an RSA seed based on a buffer and the length of the
        /// keys' modulus it will allow generating.
        ///
        /// Note that this version of the constructor duplicates the buffer.
        explicit
        Seed(elle::Buffer const& buffer,
             elle::Natural32 const length);
        /// Construct an RSA seed as above but by transferring the ownership
        /// of the buffer.
        explicit
        Seed(elle::Buffer&& buffer,
             elle::Natural32 const length);
        Seed(Seed const& seed);
        Seed(Seed&& other);
        ELLE_SERIALIZE_CONSTRUCT_DECLARE(Seed);

        /*----------.
        | Operators |
        `----------*/
      public:
        elle::Boolean
        operator ==(Seed const& other) const;
        ELLE_OPERATOR_NEQ(Seed);
        ELLE_OPERATOR_NO_ASSIGNMENT(Seed);

        /*-----------.
        | Interfaces |
        `-----------*/
      public:
        // seed
        virtual
        elle::Boolean
        operator ==(cryptography::seed::Interface const& other) const;
        virtual
        cryptography::seed::Interface*
        clone() const;
        virtual
        Cryptosystem
        cryptosystem() const;
        virtual
        elle::Natural32
        length() const;
        // printable
        virtual
        void
        print(std::ostream& stream) const;
        // serializable
        ELLE_SERIALIZE_FRIEND_FOR(Seed);

        /*-----------.
        | Attributes |
        `-----------*/
      private:
        ELLE_ATTRIBUTE_R(elle::Buffer, buffer);
        // The length, in bits, of the keys' modulus one can
        // generate with this seed.
        ELLE_ATTRIBUTE(elle::Natural32, length);
      };
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
        /*----------.
        | Functions |
        `----------*/

        /// Generate a seed of the given length.
        Seed
        generate(elle::Natural32 const length);
      }
    }
  }
}

#  include <cryptography/rsa/Seed.hxx>

# endif

#endif