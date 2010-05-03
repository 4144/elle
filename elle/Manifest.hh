//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// file          /home/mycure/infinit/elle/Manifest.hh
//
// created       julien quintard   [thu mar  4 17:35:00 2010]
// updated       julien quintard   [sun may  2 20:55:34 2010]
//

#ifndef ELLE_MANIFEST_HH
#define ELLE_MANIFEST_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/core/Natural.hh>
#include <elle/core/Character.hh>

#include <elle/standalone/Report.hh>

#include <elle/network/Tag.hh>
#include <elle/network/Range.hh>
#include <elle/network/Message.hh>
#include <elle/network/Parameters.hh>

//
// ---------- constants -------------------------------------------------------
//

namespace elle
{

  ///
  /// XXX
  ///
  extern const Character	Component[];

  ///
  /// XXX
  ///
  const Natural32		Tags = 2;

}

//
// ---------- range -----------------------------------------------------------
//

///
/// XXX
///
range(elle::Component, elle::Tags);

//
// ---------- tags ------------------------------------------------------------
//

namespace elle
{

  //
  // enumerations
  //
  enum Tag
    {
      // error
      TagError = elle::network::Range<Component>::First
    };

}

//
// ---------- manifests -------------------------------------------------------
//

///
/// below are the definitions of the messages common to every component
/// relying on the Elle library.
///

// Error
outward(elle::TagError,
	parameters(const elle::standalone::Report));

#endif
