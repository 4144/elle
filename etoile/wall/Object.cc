//
// ---------- header ----------------------------------------------------------
//
// project       etoile
//
// license       infinit
//
// file          /home/mycure/infinit/etoile/wall/Object.cc
//
// created       julien quintard   [wed mar  3 20:50:57 2010]
// updated       julien quintard   [wed apr  7 00:38:13 2010]
//

//
// ---------- includes --------------------------------------------------------
//

#include <etoile/wall/Object.hh>

namespace etoile
{
  namespace wall
  {

//
// ---------- methods ---------------------------------------------------------
//

    ///
    /// this method loads an object in a context and returns the context
    /// identifier.
    ///
    Status		Object::Load(const
				       path::Way&		way)
    {
      context::Object*		context;
      context::Identifier	identifier;
      user::User*		user;

      enter(instance(context));

      printf("[XXX] Object::Load()\n");

      // load the current user.
      if (user::User::Instance(user) == StatusError)
	escape("unable to load the user");

      // check if the user is an application..
      if (user->type != user::User::TypeApplication)
	escape("non-applications cannot authenticate");

      // allocate a new context.
      context = new context::Object;

      // create a route from the given way.
      if (context->route.Create(way) == StatusError)
	escape("unable to create a route");

      // resolve the route in an object address.
      if (path::Path::Resolve(context->route, context->address) == StatusError)
	escape("unable to resolve the given route into an object's address");

      // load the object in the given context.
      if (components::Object::Load(context, context->address) == StatusError)
	escape("unable to load the object in the given context");

      // generate an identifier.
      if (identifier.Generate() == StatusError)
	escape("unable to generate an identifier");

      // store the context in the container.
      if (context::Context::Add(identifier, context) == StatusError)
	escape("unable to store the context");

      // waive the context.
      waive(context);

      // return the context identifier to the caller.
      if (user->application->channel->Reply(
            Inputs<TagIdentifier>(identifier)) == StatusError)
	escape("unable to reply to the application");

      leave();
    }

    ///
    /// this method locks the object the identified context is related
    /// to.
    ///
    /// the method returns true if the lock has been acquired or false
    /// otherwise.
    ///
    Status		Object::Lock(const
				       context::Identifier&	identifier)
    {
      enter();

      printf("[XXX] Object::Lock()\n");

      leave();
    }

    ///
    /// this method releases a previously locked object.
    ///
    Status		Object::Release(const
					  context::Identifier&	identifer)
    {
      enter();

      printf("[XXX] Object::Release()\n");

      leave();
    }

    ///
    /// this method returns information on the object in a compact format.
    ///
    Status		Object::Information(const
					      context::Identifier& identifier)
    {
      context::Object*		context;
      State			state;
      user::User*		user;

      enter();

      printf("[XXX] Object::Information()\n");

      // load the current user.
      if (user::User::Instance(user) == StatusError)
	escape("unable to load the user");

      // check if the user is an application..
      if (user->type != user::User::TypeApplication)
	escape("non-applications cannot authenticate");

      // retrieve the context.
      if (context::Context::Retrieve(identifier, context) == StatusError)
	escape("unable to retrieve the object context");

      // check if the context is an object.
      if ((context->format & context::FormatObject) !=
	  context::FormatObject)
	escape("unable to get information on non-object contexts");

      // request the object component to fill the state structure.
      if (components::Object::Information(context, state) == StatusError)
	escape("unable to retrieve information on the object");

      // return the state to the caller.
      if (user->application->channel->Reply(
	    Inputs<TagObjectState>(state)) == StatusError)
	escape("unable to reply to the application");

      leave();
    }

    ///
    /// this method commits the modifications pending on the context
    /// and closes it.
    ///
    Status		Object::Store(const
				        context::Identifier&	identifier)
    {
      context::Object*		context;
      user::User*		user;

      enter();

      printf("[XXX] Object::Store()\n");

      // load the current user.
      if (user::User::Instance(user) == StatusError)
	escape("unable to load the user");

      // check if the user is an application..
      if (user->type != user::User::TypeApplication)
	escape("non-applications cannot authenticate");

      // retrieve the context.
      if (context::Context::Retrieve(identifier, context) == StatusError)
	escape("unable to retrieve the object context");

      // check if the context is an object.
      if ((context->format & context::FormatObject) !=
	  context::FormatObject)
	escape("unable to store non-object contexts");

      // store the context.
      if (components::Object::Store(context) == StatusError)
	escape("unable to store the object context");

      // reply to the application.
      if (user->application->channel->Reply(Inputs<TagOk>()) == StatusError)
	escape("unable to reply to the application");

      leave();
    }

  }
}
