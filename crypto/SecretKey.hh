//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit (c)
//
// file          /home/mycure/infinit/elle/crypto/SecretKey.hh
//
// created       julien quintard   [thu nov  1 11:38:37 2007]
// updated       julien quintard   [fri feb  5 02:08:24 2010]
//

#ifndef ELLE_CRYPTO_SECRETKEY_HH
#define ELLE_CRYPTO_SECRETKEY_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/io/IO.hh>

#include <elle/core/Core.hh>

#include <elle/misc/Misc.hh>

#include <elle/archive/Archive.hh>

#include <elle/crypto/Plain.hh>
#include <elle/crypto/Cipher.hh>
#include <elle/crypto/Clear.hh>
#include <elle/crypto/Digest.hh>
#include <elle/crypto/OneWay.hh>

#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <fcntl.h>

namespace elle
{
  using namespace io;
  using namespace core;
  using namespace misc;
  using namespace archive;

  namespace crypto
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// this class represents a secret key for symmetric encryption.
    ///
    class SecretKey:
      public Entity,
      public Dumpable, public Archivable
    {
    public:
      //
      // constants
      //
      static const Character		Magic[];

      struct Default
      {
	static const Natural32		Length;
      };

      struct Algorithms
      {
	static const ::EVP_CIPHER*	Cipher;
	static const ::EVP_MD*		Digest;
      };

      //
      // methods
      //
      Status		Create(const String&);

      Status		Generate();
      Status		Generate(const Natural32);

      Status		Encrypt(const Plain&,
				Cipher&) const;
      Status		Decrypt(const Cipher&,
				Clear&) const;

      template <typename T>
      Status		Decrypt(const Cipher&,
				T&) const;
      template <typename T,
		typename... TT>
      Status		Decrypt(const Cipher&,
				T&,
				TT&...) const;

      //
      // interfaces
      //

      // entity
      Embed(Entity, SecretKey);
      Boolean		operator==(const SecretKey&) const;

      // dumpable
      Status		Dump(const Natural32 = 0) const;

      // archivable
      Status		Serialize(Archive&) const;
      Status		Extract(Archive&);

      //
      // attributes
      //
      Region		key;

      //
      // forward methods
      //

      ///
      /// this methods are required because the compiler, given an Archive
      /// object will call a template-based method instead of the Plain one.
      ///
      /// we do not want this especially because the template-based methods
      /// build archives and we are already receiving an archive.
      ///

      Status		Encrypt(const Archive&		archive,
				Cipher&			cipher) const
      {
	return (this->Encrypt((Plain&)archive, cipher));
      }

      Status		Decrypt(const Cipher&		cipher,
				Archive&		archive) const
      {
	return (this->Decrypt(cipher, (Plain&)archive));
      }

      //
      // variadic templates
      //

      // encrypt
      template <typename T1>
      Status		Encrypt(const T1&,
				Cipher&) const;
      template <typename T1,
		typename T2>
      Status		Encrypt(const T1&,
				const T2&,
				Cipher&) const;
      template <typename T1,
		typename T2,
		typename T3>
      Status		Encrypt(const T1&,
				const T2&,
				const T3&,
				Cipher&) const;
      template <typename T1,
		typename T2,
		typename T3,
		typename T4>
      Status		Encrypt(const T1&,
				const T2&,
				const T3&,
				const T4&,
				Cipher&) const;
      template <typename T1,
		typename T2,
		typename T3,
		typename T4,
		typename T5>
      Status		Encrypt(const T1&,
				const T2&,
				const T3&,
				const T4&,
				const T5&,
				Cipher&) const;
      template <typename T1,
		typename T2,
		typename T3,
		typename T4,
		typename T5,
		typename T6>
      Status		Encrypt(const T1&,
				const T2&,
				const T3&,
				const T4&,
				const T5&,
				const T6&,
				Cipher&) const;
      template <typename T1,
		typename T2,
		typename T3,
		typename T4,
		typename T5,
		typename T6,
		typename T7>
      Status		Encrypt(const T1&,
				const T2&,
				const T3&,
				const T4&,
				const T5&,
				const T6&,
				const T7&,
				Cipher&) const;
      template <typename T1,
		typename T2,
		typename T3,
		typename T4,
		typename T5,
		typename T6,
		typename T7,
		typename T8>
      Status		Encrypt(const T1&,
				const T2&,
				const T3&,
				const T4&,
				const T5&,
				const T6&,
				const T7&,
				const T8&,
				Cipher&) const;
      template <typename T1,
		typename T2,
		typename T3,
		typename T4,
		typename T5,
		typename T6,
		typename T7,
		typename T8,
		typename T9>
      Status		Encrypt(const T1&,
				const T2&,
				const T3&,
				const T4&,
				const T5&,
				const T6&,
				const T7&,
				const T8&,
				const T9&,
				Cipher&) const;
    };

  }
}

//
// ---------- templates -------------------------------------------------------
//

#include <elle/crypto/SecretKey.hxx>

#endif
