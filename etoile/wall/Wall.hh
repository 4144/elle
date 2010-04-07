//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit (c)
//
// file          /home/mycure/infinit/etoile/wall/Wall.hh
//
// created       julien quintard   [fri aug 14 12:56:07 2009]
// updated       julien quintard   [tue apr  6 12:59:24 2010]
//

#ifndef ETOILE_WALL_WALL_HH
#define ETOILE_WALL_WALL_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/Elle.hh>

#include <etoile/Manifest.hh>

namespace etoile
{
  namespace wall
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// the Wal method deal with identifying, authenticating and connecting
    /// clients so that once authenticated/connected, an application can
    /// issue requests.
    ///
    class Wall
    {
    public:
      //
      // constants
      //
      static const String&		Line;

      //
      // static methods
      //
      static Status	Initialize();
      static Status	Clean();

      static Status	Identify(const PublicKey&);
      static Status	Authenticate(const Digest&);
      static Status	Connect(const String&);

      //
      // callbacks
      //
      static Status	Error(const Report&);
      static Status	Connection(Door*&);
    };

  }
}

//
// ---------- includes --------------------------------------------------------
//

#include <etoile/configuration/Configuration.hh>
#include <etoile/user/User.hh>

//
// ---------- includes --------------------------------------------------------
//

#include <etoile/wall/Interface.hh>
#include <etoile/wall/Types.hh>
#include <etoile/wall/User.hh>
#include <etoile/wall/Object.hh>
#include <etoile/wall/Directory.hh>

#endif
