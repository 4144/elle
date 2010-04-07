//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/journal/Journal.cc
//
// created       julien quintard   [sat jan 30 15:22:54 2010]
// updated       julien quintard   [tue apr  6 12:13:53 2010]
//

//
// ---------- includes --------------------------------------------------------
//

#include <etoile/journal/Journal.hh>

namespace etoile
{
  namespace journal
  {

//
// ---------- static methods --------------------------------------------------
//

    ///
    /// this method initializes the journal.
    ///
    Status		Journal::Initialize()
    {
      enter();

      // nothing to do

      leave();
    }

    ///
    /// this method cleans the journal.
    ///
    Status		Journal::Clean()
    {
      enter();

      // nothing to do.

      leave();
    }

    ///
    /// this method registers a context as being ready to be processed.
    ///
    Status		Journal::Record(context::Context*	context)
    {
      Bucket::Scoutor	scoutor;

      enter();

      // XXX easy temporary version, just publish everything.

      // go through the blocks and publish/destroy them.
      for (scoutor = context->bucket.container.begin();
	   scoutor != context->bucket.container.end();
	   scoutor++)
	{
	  Item*		item = *scoutor;

	  switch (item->operation)
	    {
	    case OperationPush:
	      {
		if (hole::Hole::Put(item->block->address,
				    item->block) == StatusError)
		  escape("unable to publish the block through hole");

		break;
	      }
	    case OperationPop:
	      {
		if (hole::Hole::Destroy(*item->address) == StatusError)
		  escape("unable to destroy the block through hole");

		break;
	      }
	    }
	}

      leave();
    }

    ///
    /// this method is called by Depot whenever looking for a particular
    /// block.
    ///
    Status		Journal::Get(const hole::Address&	address,
				     hole::Block*&		block)
    {
      enter();

      // XXX

      leave();
    }

  }
}
