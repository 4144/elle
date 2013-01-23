#include <etoile/automaton/Author.hh>
#include <etoile/automaton/Rights.hh>
#include <etoile/automaton/Access.hh>
#include <etoile/gear/Object.hh>

#include <nucleus/neutron/Author.hh>
#include <nucleus/neutron/Index.hh>
#include <nucleus/neutron/Access.hh>
#include <nucleus/neutron/Object.hh>

#include <agent/Agent.hh>

namespace etoile
{
  namespace automaton
  {

//
// ---------- methods ---------------------------------------------------------
//

    ///
    /// this method forges the author objects require in order to prove
    /// the user had the permission to operate upon the object.
    ///
    elle::Status        Author::Forge(gear::Object&             context)
    {
      // if an author exists, return.
      if (context.author != nullptr)
        return elle::Status::Ok;

      // determine the rights.
      if (Rights::Determine(context) == elle::Status::Error)
        escape("unable to determine the rights");

      // build the author object according to the subject's role.
      switch (context.rights.role)
        {
        case nucleus::neutron::Object::RoleOwner:
          {
            // Create an owner author.
            context.author = new nucleus::neutron::Author;

            break;
          }
        case nucleus::neutron::Object::RoleLord:
          {
            // open the access.
            if (Access::Open(context) == elle::Status::Error)
              escape("unable to open the access");

            // lookup the user's subject in the access records.
            nucleus::neutron::Index index{
              context.access->seek(agent::Agent::Subject)};

            // Create a lord author.
            context.author = new nucleus::neutron::Author(index);

            break;
          }
        default:
          {
            // XXX to implement.
            assert(false);

            break;
          }
        }

      assert(context.author != nullptr);

      return elle::Status::Ok;
    }

  }
}
