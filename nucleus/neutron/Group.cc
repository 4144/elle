#include <nucleus/neutron/Group.hh>
#include <nucleus/neutron/Fellow.hh>
#include <nucleus/Exception.hh>

#include <cryptography/KeyPair.hh>
#include <cryptography/PrivateKey.hh>

#include <elle/idiom/Open.hh>

namespace nucleus
{
  namespace neutron
  {

    /*----------.
    | Constants |
    `----------*/

    elle::Natural32 const Group::Constants::keypair_length{1024};
    Component const Group::Constants::component{ComponentGroup};

//
// ---------- construction ----------------------------------------------------
//

    Group::Group():
      ImprintBlock::ImprintBlock(),

      _size(1),
      _manager_fellow(nullptr)
    {
    }

    Group::Group(proton::Network const& network,
                 cryptography::PublicKey const& manager_K,
                 elle::String const& description):
      ImprintBlock::ImprintBlock(network, ComponentGroup, manager_K),

      _description(description),
      _size(1),
      _manager_fellow(nullptr)
    {
      /* XXX[this is a hack which consists in generating a unique pass
             which will not evolve over time. */
      {
        cryptography::KeyPair pass(
          cryptography::KeyPair::generate(Group::Constants::keypair_length));

        Token token(pass.k(), this->manager_subject().user());
        this->upgrade(proton::Address::null(), pass.K(), token);
      }
    }

    Group::~Group()
    {
      delete this->_manager_fellow;
    }

//
// ---------- methods ---------------------------------------------------------
//

    void
    Group::upgrade(proton::Address const& ensemble,
                   cryptography::PublicKey const& pass_K,
                   Token const& manager_token)
    {
      this->_pass_K = pass_K;

      if (this->_modification_timestamp.Current() == elle::Status::Error)
        throw Exception("unable to set the last management time"); // XXX[to remove later]

      this->_ensemble = ensemble;
      this->_manager_token = manager_token;

      this->state(proton::State::dirty);
    }

    void
    Group::downgrade()
    {
      if (this->_modification_timestamp.Current() == elle::Status::Error)
        throw Exception("unable to set the last management time"); // XXX[to remove later]

      this->_ensemble = proton::Address::null();

      this->state(proton::State::dirty);
    }

    void
    Group::seal(cryptography::PrivateKey const& owner_k)
    {
      switch (this->state())
        {
        case proton::State::clean:
        case proton::State::consistent:
          {
            // Nothing to do, the group is already valid.

            break;
          }
        case proton::State::dirty:
          {
            this->_signature =
              owner_k.sign(elle::serialize::make_tuple(
                             this->_description,
                             this->_pass_K,
                             this->_size,
                             this->_modification_timestamp,
                             this->_ensemble,
                             this->_manager_token));

            // Increase the mutable block's revision.
            this->revision(this->revision() + 1);

            break;
          }
        }

      // Now that the group has been sealed, the block can be considered
      // as consistent.
      this->state(proton::State::consistent);
    }

    cryptography::PublicKey const&
    Group::manager_K() const
    {
      return (this->owner_K());
    }

    Subject const&
    Group::manager_subject()
    {
      // The manager is also the owner of the block. In other words, both terms
      // are synonyms. Therefore, this method returns the underylying physical
      // block's owner subject.
      return (this->owner_subject());
    }

    Fellow&
    Group::manager_fellow()
    {
      // Create the fellow corresponding to the group manager, if necessary.
      // Note that this fellow will never be serialized but is used to ease
      // the process of group management since most method manipulate fellows.
      if (this->_manager_fellow == nullptr)
        this->_manager_fellow = new Fellow(this->manager_subject(),
                                           this->_manager_token);

      assert (this->_manager_fellow != nullptr);

      return (*this->_manager_fellow);
    }

//
// ---------- dumpable --------------------------------------------------------
//

    elle::Status
    Group::Dump(const elle::Natural32 margin) const
    {
      elle::String alignment(margin, ' ');

      std::cout << alignment << "[Group]" << std::endl;

      if (ImprintBlock::Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the underlying block");

      std::cout << alignment << elle::io::Dumpable::Shift
                << "[Description] " << this->_description << std::endl;

      if (this->_pass_K.Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the public pass");

      std::cout << alignment << elle::io::Dumpable::Shift
                << "[Size] " << std::dec << this->_size << std::endl;

      std::cout << alignment << elle::io::Dumpable::Shift
                << "[Modifcation Time]" << std::endl;

      if (this->_modification_timestamp.Dump(margin + 4) == elle::Status::Error)
        escape("unable to dump the timestamp");

      if (this->_ensemble.Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the ensemble's address");

      if (this->_manager_token.Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the token");

      if (this->_manager_fellow != nullptr)
        {
          if (this->_manager_fellow->Dump(margin + 2) == elle::Status::Error)
            escape("unable to dump the fellow");
        }
      else
        {
          std::cout << alignment << elle::io::Dumpable::Shift
                    << elle::io::Dumpable::Shift << "[Fellow] "
                    << elle::none << std::endl;
        }

      if (this->_signature.Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the signature");

      return (elle::Status::Ok);
    }

//
// ---------- printable -------------------------------------------------------
//

    void
    Group::print(std::ostream& stream) const
    {
      stream << "group()";
    }

  }
}
