#include <etoile/automaton/Rights.hh>
#include <etoile/automaton/Access.hh>
#include <etoile/gear/Object.hh>
#include <etoile/gear/Group.hh>
#include <etoile/depot/Depot.hh>

#include <nucleus/proton/Revision.hh>
#include <nucleus/neutron/Token.hh>
#include <nucleus/neutron/Record.hh>
#include <nucleus/neutron/Permissions.hh>
#include <nucleus/neutron/Access.hh>
#include <nucleus/neutron/Object.hh>
#include <nucleus/neutron/Group.hh>
#include <nucleus/neutron/Ensemble.hh>

#include <cryptography/SecretKey.hh>
#include <cryptography/PrivateKey.hh>
// XXX[temporary: for cryptography]
using namespace infinit;

#include <agent/Agent.hh>

#include <elle/log.hh>

ELLE_LOG_COMPONENT("infinit.etoile.automaton.Rights");

namespace etoile
{
  namespace automaton
  {

    ///
    /// this method determines the rights the current user has over the
    /// given context.
    ///
    /// note that should have the rights already been determined, the
    /// method returns. therefore, this method can be called successively.
    ///
    elle::Status        Rights::Determine(
                          gear::Object&                         context)
    {
      ELLE_TRACE_FUNCTION(context);

      if (context.rights.role != nucleus::neutron::Object::RoleUnknown)
        {
          ELLE_TRACE("rights have already been determined")
          return elle::Status::Ok;
        }

      // determine the rights according to the subject.
      if (agent::Agent::Subject == context.object->owner_subject())
        {
          //
          // if the user is the object's owner, retrieve the user's
          // permissions, token etc. from the object's meta section.
          //

          ELLE_TRACE("the user is the object owner");

          // set the role.
          context.rights.role = nucleus::neutron::Object::RoleOwner;

          // set the permissions.
          context.rights.permissions = context.object->owner_permissions();

          // if a token is present, decrypt it.
          if (context.object->owner_token() != nucleus::neutron::Token::null())
            {
              // extract the secret key from the token.
              delete context.rights.key;
              context.rights.key =
                new cryptography::SecretKey{
                  context.object->owner_token().
                    extract<cryptography::SecretKey>(
                      agent::Agent::Identity.pair().k())};
            }

          // set the record for ease purpose.
          delete context.rights.record;
          context.rights.record =
            new nucleus::neutron::Record(context.object->owner_record());
        }
      else
        {
          //
          // if the user is not the owner, open the access block and
          // retrieve the permissions, token etc. from the access record
          // associated with the subject.
          //

          ELLE_TRACE("the user is _not_ the object owner");

          // open the access.
          if (Access::Open(context) == elle::Status::Error)
            escape("unable to open the access block");

          // check that the subject is referenced in the access block.
          if (context.access->exist(agent::Agent::Subject) == true)
            {
              //
              // in this case, the subject is referenced in the ACL, hence
              // is considered a lord.
              //

              ELLE_TRACE("the subject seems to be present in "
                         "the Access block");

              // retrieve the record associated with this subject.
              nucleus::neutron::Record const& record =
                context.access->locate(agent::Agent::Subject);

              // set the role.
              context.rights.role = nucleus::neutron::Object::RoleLord;

              // set the permissions according to the access record.
              context.rights.permissions = record.permissions();

              // finally, set the record for ease purpose.
              delete context.rights.record;
              context.rights.record =
                new nucleus::neutron::Record(record);

              // if a token is present, decrypt it.
              if (context.rights.record->token() != nullptr)
                {
                  // extract the secret key from the token.
                  delete context.rights.key;
                  context.rights.key =
                    new cryptography::SecretKey{
                      context.rights.record->token()->
                        extract<cryptography::SecretKey>(
                          agent::Agent::Identity.pair().k())};
                }
            }
          else
            {
              //
              // the subject seems to be a vassal of some sort i.e either
              // referenced by a Group or referenced nowhere but possessing
              // a token.
              //

              ELLE_TRACE("the subject does _not_ seem to be present "
                             "in the Access block: look in the groups");

              // Go through the Access records in order to explore the groups
              // which have been granted some permissions over the object.
              for (auto& pair: *context.access)
                {
                  auto& record = pair.second;

                  if (record->subject().type() ==
                      nucleus::neutron::Subject::TypeGroup)
                    {
                      std::unique_ptr<nucleus::neutron::Group> group;

                      ELLE_TRACE_SCOPE("looking at the group '%s'",
                                       record->subject().group());

                      // XXX[remove try/catch later]
                      try
                        {
                          // Retrieve the group block.
                          group =
                            depot::Depot::pull_group(
                              record->subject().group(),
                              nucleus::proton::Revision::Last);
                        }
                      catch (std::exception const& e)
                        {
                          escape("%s", e.what());
                        }

                      // Check if the subject is actually the group manager.
                      if (agent::Agent::Subject == group->manager_subject())
                        {
                          ELLE_TRACE("the subject is the manager of "
                                     "the group '%s'",
                                     record->subject().group());

                          context.rights.role = nucleus::neutron::Object::RoleVassal;
                          context.rights.permissions = record->permissions();

                          // Compute a token with the appropriate information taken
                          // from the group and the access record.
                          delete context.rights.record;
                          context.rights.record =
                            new nucleus::neutron::Record(
                              group->manager_subject(),
                              record->permissions(),
                              record->token());

                          // Finally, extract the key from the record so as
                          // to be able to decrypt the object's content, should
                          // a token be present though.
                          if (context.rights.record->token() != nullptr)
                            {
                              ELLE_TRACE("the access token is present");

                              ELLE_TRACE("decrypting the private pass");

                              nucleus::neutron::Token token =
                                group->manager_token();

                              // First, extract the private pass from the
                              // manager's fellow.
                              cryptography::PrivateKey pass_k =
                                token.extract<cryptography::PrivateKey>(
                                  agent::Agent::Identity.pair().k());

                              ELLE_TRACE("decrypting the access token");

                              // With the private pass, one can decrypt the
                              // access token associated with the group.
                              delete context.rights.key;
                              context.rights.key =
                                new cryptography::SecretKey{
                                  context.rights.record->token()->extract<cryptography::SecretKey>(
                                    pass_k)};
                            }
                          else
                            {
                              ELLE_TRACE("the access token is _not_ present");
                            }
                        }
                      else
                        {
                          // In this case, the subject is not the group manager. We must
                          // therefore look in the group's ensemble so as to locate the
                          // subject's fellow entry.

                          ELLE_TRACE("the subject is _not_ the group manager");

                          if (group->ensemble() != nucleus::proton::Address::null())
                            {
                              std::unique_ptr<nucleus::neutron::Ensemble> ensemble;

                              ELLE_TRACE_SCOPE("the Ensemble block is present: lookup the subject");

                              // XXX[remove try/catch later]
                              try
                                {
                                  // Retrieve the ensemble which contains the list of
                                  // the subjects belonging to the group.
                                  ensemble =
                                    depot::Depot::pull_ensemble(group->ensemble());
                                }
                              catch (std::exception const& e)
                                {
                                  escape("%s", e.what());
                                }

                              // Look for the user's subject in the ensemble.
                              if (ensemble->exist(agent::Agent::Subject) == false)
                                {
                                  ELLE_TRACE("the subject does not exist in the ensemble");
                                  continue;
                                }

                              // XXX[remove the try/catch later]
                              try
                                {
                                  nucleus::neutron::Fellow const& fellow =
                                    ensemble->locate(agent::Agent::Subject);

                                  context.rights.role =
                                    nucleus::neutron::Object::RoleVassal;
                                  context.rights.permissions =
                                    record->permissions();

                                  // Compute a token with the appropriate
                                  // information taken from the group and the
                                  // access record.
                                  delete context.rights.record;
                                  context.rights.record =
                                    new nucleus::neutron::Record(
                                      fellow.subject(),
                                      record->permissions(),
                                      record->token());

                                  // Finally, extract the key from the record so
                                  // as to be able to decrypt the object's
                                  // content, should a token be present though.
                                  if (context.rights.record->token() != nullptr)
                                    {
                                      ELLE_TRACE("the access token is present");

                                      ELLE_TRACE("decrypting the private pass");

                                      nucleus::neutron::Token token = fellow.token();

                                      // First, extract the private pass
                                      // from the fellow.
                                      cryptography::PrivateKey pass_k =
                                        token.extract<cryptography::PrivateKey>(
                                          agent::Agent::Identity.pair().k());

                                      // With the private pass, one can decrypt
                                      // the access token associated with the
                                      // group.
                                      delete context.rights.key;
                                      context.rights.key =
                                        new cryptography::SecretKey{
                                          context.rights.record->token()->extract<cryptography::SecretKey>(pass_k)};
                                    }
                                  else
                                    {
                                      ELLE_TRACE("the access token is _not_ present");
                                    }
                                }
                              catch (std::exception const& e)
                                {
                                  escape("%s", e.what());
                                }
                            }
                          else
                            {
                              ELLE_TRACE("the Ensemble block is _not_ present: skip this group");
                            }
                        }
                    }
                }

              // if the system failed at defining the user's role, one can
              // conclude that the user has no role and no permission over
              // the object.
              if (context.rights.role == nucleus::neutron::Object::RoleUnknown)
                {
                  ELLE_TRACE("the user has no rights over the object");

                  context.rights.role = nucleus::neutron::Object::RoleNone;
                  context.rights.permissions = nucleus::neutron::permissions::none;
                }
              else
                {
                  ELLE_TRACE("the user plays the role of %s and has "
                                 "the %s permissions over the object",
                                 context.rights.role, context.rights.permissions);
                }
            }
        }

      return elle::Status::Ok;
    }

    ///
    /// this method is triggered whenever the user's permissions have
    /// been changed.
    ///
    /// note however that the token, key etc. are not being changed.
    ///
    elle::Status        Rights::Update(
                          gear::Object&                         context,
                          const nucleus::neutron::Permissions& permissions)
    {
      ELLE_TRACE_FUNCTION(context, permissions);

      // update the permission.
      context.rights.permissions = permissions;

      // also update the record which also include the user's permission.
      ELLE_ASSERT(context.rights.record != nullptr);
      context.rights.record->permissions(permissions);

      return elle::Status::Ok;
    }

    elle::Status
    Rights::Determine(gear::Group& context)
    {
      ELLE_TRACE_FUNCTION(context);

      if (context.rights.role != nucleus::neutron::Group::RoleUnknown)
        {
          ELLE_TRACE("rights have already been determined")

          return elle::Status::Ok;
        }

      // determine the rights according to the subject.
      if (agent::Agent::Subject == context.group->manager_subject())
        {
          //
          // if the user is the group's manager, retrieve the user's
          // token.
          //

          ELLE_TRACE_SCOPE("the user is the group manager");

          context.rights.role = nucleus::neutron::Group::RoleManager;
        }
      else
        {
          //
          // if the user is not the manager, leave the role as unknown.
          //

          ELLE_TRACE_SCOPE("the user is _not_ the group manager");

          context.rights.role = nucleus::neutron::Group::RoleNone;
        }

      return (elle::Status::Ok);
    }

  }
}
