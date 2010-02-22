//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// file          /home/mycure/infinit/elle/network/Slot.hh
//
// created       julien quintard   [wed feb  3 21:04:37 2010]
// updated       julien quintard   [mon feb 22 13:30:18 2010]
//

#ifndef ELLE_NETWORK_SLOT_HH
#define ELLE_NETWORK_SLOT_HH

//
// ---------- includes --------------------------------------------------------
//

#include <elle/core/Core.hh>

#include <elle/network/Socket.hh>
#include <elle/network/Port.hh>
#include <elle/network/Address.hh>
#include <elle/network/Message.hh>
#include <elle/network/Environment.hh>
#include <elle/network/Network.hh>

#include <QObject>
#include <QUdpSocket>

namespace elle
{
  namespace network
  {

//
// ---------- classes ---------------------------------------------------------
//

    ///
    /// a slot represents a non-connected socket, typically implemented
    /// through the UDP protocol.
    ///
    class Slot:
      public ::QObject,
      public Socket
    {
      Q_OBJECT;

    public:
      //
      // constructors & destructors
      //
      Slot();
      ~Slot();

      //
      // methods
      //
      Status		Create();
      Status		Create(const Port);

      template <const Tag G, typename... T>
      Status		Send(const Address&,
			     const T&...);

      //
      // attributes
      //
      ::QUdpSocket*	socket;
      Port		port;

    private slots:
      //
      // slots
      //
      void		Read();
    };

  }
}

//
// ---------- templates -------------------------------------------------------
//

#include <elle/network/Slot.hxx>

#endif

