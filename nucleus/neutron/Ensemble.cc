#include <nucleus/neutron/Ensemble.hh>
#include <nucleus/Exception.hh>

#include <cryptography/PrivateKey.hh>
#include <elle/log.hh>

#include <elle/idiom/Open.hh>

ELLE_LOG_COMPONENT("infinit.nucleus.neutron.Ensemble");

namespace nucleus
{
  namespace neutron
  {

//
// ---------- constants -------------------------------------------------------
//

    const Component Ensemble::component = ComponentEnsemble;

//
// ---------- construction ----------------------------------------------------
//

    Ensemble::Ensemble():
      proton::ContentHashBlock()
    {
    }

    Ensemble::Ensemble(proton::Network const& network,
                       cryptography::PublicKey const& creator_K):
      proton::ContentHashBlock(network, ComponentEnsemble, creator_K)
    {
    }

//
// ---------- methods ---------------------------------------------------------
//

    void
    Ensemble::add(std::unique_ptr<Fellow>&& fellow)
    {
      ELLE_TRACE_SCOPE("[%p] add(%s)", this, fellow);

      if (this->exists(fellow->subject()) == true)
        throw Exception("a fellow with this subject already exists");
        // XXX[remove in the future]

      this->_container.push_back(fellow.release());

      this->state(proton::StateDirty);
    }

    elle::Boolean
    Ensemble::empty() const
    {
      return (this->_container.empty());
    }

    elle::Boolean
    Ensemble::exists(Subject const& subject) const
    {
      Ensemble::Scoutor scoutor;

      ELLE_TRACE_SCOPE("[%p] exists(%s)", this, subject);

      try
        {
          scoutor = this->_locate(subject);
        }
      catch (Exception const& e)
        {
          return (false);
        }

      return (true);
    }

    Fellow const&
    Ensemble::locate(Subject const& subject) const
    {
      Ensemble::Scoutor scoutor;
      Fellow* fellow;

      ELLE_TRACE_SCOPE("[%p] locate(%s)", this, subject);

      scoutor = this->_locate(subject);
      fellow = *scoutor;

      return (*fellow);
    }

    Fellow const&
    Ensemble::locate(Index const& index) const
    {
      Ensemble::Scoutor scoutor;
      Index i(0);

      ELLE_TRACE_SCOPE("[%p] locate(%s)", this, index);

      for (scoutor = this->_container.begin();
           scoutor != this->_container.end();
           ++scoutor, ++i)
        {
          Fellow* fellow = *scoutor;

          if (index == i)
            return (*fellow);
        }

      throw Exception(
              elle::sprintf(
                "unable to locate the fellow at the given index %s",
                index));
    }

    Index
    Ensemble::seek(Subject const& subject) const
    {
      Ensemble::Scoutor scoutor;
      Index i(0);

      ELLE_TRACE_SCOPE("[%p] seek(%s)", this, subject);

      for (scoutor = this->_container.begin();
           scoutor != this->_container.end();
           ++scoutor, ++i)
        {
          Fellow* fellow = *scoutor;

          if (fellow->subject() == subject)
            return (i);
        }

      throw Exception(
              elle::sprintf(
                "unable to locate the fellow at the given index %s",
                subject));
    }

    Range<Fellow> const
    Ensemble::consult(Index const& index,
                      Size const& size) const
    {
      Ensemble::Scoutor scoutor;
      Range<Fellow> range;
      Index i(0);

      ELLE_TRACE_SCOPE("consult(%s, %s)", index, size);

      /* XXX
      if (range.Detach() == elle::Status::Error)
        throw Exception("unable to detach the data from the range"); // XXX[to remove in the future]
      */

      for (scoutor = this->_container.begin();
           scoutor != this->_container.end();
           ++scoutor)
        {
          Fellow* fellow = *scoutor;

          // If this record lies in the selected range [index, index + size[.
          if ((i >= index) && (i < (index + size)))
            {
              if (range.Add(fellow) == elle::Status::Error)
                throw Exception("unable to add the record to the given range");
                // XXX[to remove in the future]
            }
        }

      return (range);
    }

    void
    Ensemble::update(cryptography::PrivateKey const& pass_k)
    {
      Ensemble::Iterator iterator;

      ELLE_TRACE_SCOPE("[%p] update(%s)", this, pass_k);

      for (iterator = this->_container.begin();
           iterator != this->_container.end();
           ++iterator)
        {
          Fellow* fellow = *iterator;

          switch (fellow->subject().type())
            {
            case Subject::TypeUser:
              {
                ELLE_TRACE_SCOPE("update fellow user '%s'", fellow->subject());

                // Update the fellow's token with the freshly constructed
                // token which embeds the new private key encrypted with the
                // user's public key so that only he can decrypt it.
                fellow->token(Token(pass_k, fellow->subject().user()));

                break;
              }
            case Subject::TypeGroup:
              {
                // XXX
                throw Exception("not yet implemented");

                break;
              }
            case Subject::TypeUnknown:
            default:
              {
                throw Exception(
                        elle::sprintf("unknown subject type '%s'",
                                      fellow->subject().type()));
              }
            }
        }

      this->state(proton::StateDirty);
    }

    void
    Ensemble::remove(Subject const& subject)
    {
      Iterator iterator;
      Fellow* fellow;

      iterator = this->_locate(subject);
      fellow = *iterator;

      delete fellow;

      this->_container.erase(iterator);

      this->state(proton::StateDirty);
    }

    Size
    Ensemble::size() const
    {
      return (static_cast<Size>(this->_container.size()));
    }

    Ensemble::Scoutor const
    Ensemble::_locate(Subject const& subject) const
    {
      Ensemble::Scoutor scoutor;

      for (scoutor = this->_container.begin();
           scoutor != this->_container.end();
           ++scoutor)
        {
          Fellow* fellow = *scoutor;

          if (fellow->subject() == subject)
            return (scoutor);
        }

      throw Exception(
              elle::sprintf(
                "unable to locate the given subject %s",
                subject));
    }

    Ensemble::Iterator const
    Ensemble::_locate(Subject const& subject)
    {
      Ensemble::Iterator iterator;

      for (iterator = this->_container.begin();
           iterator != this->_container.end();
           ++iterator)
        {
          Fellow* fellow = *iterator;

          if (fellow->subject() == subject)
            return (iterator);
        }

      throw Exception(elle::sprintf("unable to locate the given subject %s",
                                    subject));
    }

//
// ---------- dumpable --------------------------------------------------------
//

    elle::Status
    Ensemble::Dump(const elle::Natural32 margin) const
    {
      elle::String alignment(margin, ' ');

      std::cout << alignment << "[Ensemble] #"
                << this->_container.size() << std::endl;

      if (ContentHashBlock::Dump(margin + 2) == elle::Status::Error)
        escape("unable to dump the underlying block");

      std::cout << alignment << elle::io::Dumpable::Shift
                << "[Fellows]" << std::endl;

      for (auto fellow : this->_container)
        {
          if (fellow->Dump(margin + 4) == elle::Status::Error)
            escape("unable to dump the fellow");
        }

      return (elle::Status::Ok);
    }

//
// ---------- printable -------------------------------------------------------
//

    void
    Ensemble::print(std::ostream& stream) const
    {
      stream << "ensemble("
             << "#" << this->_container.size()
             << ")";
    }

  }
}
