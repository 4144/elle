//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// author        julien quintard   [mon jan 26 20:21:10 2009]
//

#ifndef ELLE_SYSTEM_SYSTEM_HH
#define ELLE_SYSTEM_SYSTEM_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/types.hh>
#include <elle/types.hh>

#include <elle/radix/Status.hh>

namespace elle
{

  using namespace radix;

  ///
  /// this namespace contains system-related stuff especially useful
  /// when it comes to portability.
  ///
  namespace system
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// this class contains system-wide information.
    ///
    class System
    {
    public:
      //
      // enumerations
      //

      ///
      /// this enumeration represents the endianness.
      ///
      enum Order
        {
          OrderLittle = 0x1,
          OrderBig
        };

      //
      // constants
      //
      static Order                      Endianness;

      struct Path
      {
        static Character                Separator;
        static String                   Home;
        static String                   Root;
        static String                   Current;
      };

      //
      // static methods
      //
      static Status     Initialize();
      static Status     Clean();
    };

  }
}

#endif
