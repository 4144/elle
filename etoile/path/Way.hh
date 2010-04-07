//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/path/Way.hh
//
// created       julien quintard   [tue mar 30 23:31:29 2010]
// updated       julien quintard   [wed apr  7 00:53:13 2010]
//

#ifndef ETOILE_PATH_WAY_HH
#define ETOILE_PATH_WAY_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/Elle.hh>

#include <etoile/path/Length.hh>

namespace etoile
{
  namespace path
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// this class represents a string-based path i.e a way.
    ///
    class Way:
      public Entity,
      public Dumpable, public Archivable
    {
    public:
      //
      // constructors & destructors
      //
      Way();
      Way(const Way&);
      Way(const String&);
      Way(const String&,
	  String&);

      //
      // methods
      //
      Status		Capacity(Length&) const;


      // interfaces
      //

      // entity
      declare(Entity, Way);
      // XXX operator==

      // dumpable
      Status		Dump(const Natural32 = 0) const;

      // archivable
      Status		Serialize(Archive&) const;
      Status		Extract(Archive&);

      //
      // attributes
      //
      String		path;
    };

  }
}

#endif
