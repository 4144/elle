//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// author        julien quintard   [mon jul  4 16:34:01 2011]
//

#ifndef ELLE_IO_MANIPULATORS_HH
#define ELLE_IO_MANIPULATORS_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/types.hh>
#include <elle/types.hh>

#include <elle/io/Unique.hh>
#include <elle/io/Uniquable.hh>
#include <elle/io/Format.hh>

#include <elle/idiom/Close.hh>
# include <iostream>
# include <sstream>
# include <string>
#include <elle/idiom/Open.hh>

namespace std
{

//
// ---------- functions -------------------------------------------------------
//

  elle::String    chop(const elle::String&,
                             const elle::Natural32 = 50);
  template <const elle::io::Format F>
  elle::String    chop(const elle::io::Uniquable<F>&,
                             const elle::Natural32 = 50);

}

#endif
