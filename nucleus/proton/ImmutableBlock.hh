#ifndef NUCLEUS_PROTON_IMMUTABLEBLOCK_HH
# define NUCLEUS_PROTON_IMMUTABLEBLOCK_HH

#include <elle/types.hh>
#include <elle/io/Fileable.hh>

#include <nucleus/proton/Block.hh>
#include <nucleus/proton/Address.hh>
#include <nucleus/proton/Network.hh>
#include <nucleus/proton/Family.hh>

#include <nucleus/neutron/Component.hh>

namespace nucleus
{
  namespace proton
  {

    ///
    /// this class derives the Block and abstracts the notion of
    /// immutable block.
    ///
    class ImmutableBlock
      : public Block
      , public elle::io::Fileable<ImmutableBlock>
    {
    public:
      //
      // constructors & destructors
      //
      ImmutableBlock();
      ImmutableBlock(const Family,
                     const neutron::Component);

      //
      // interfaces
      //

      // object
      declare(ImmutableBlock);

      // fileable
      elle::Status      Load(const Network&,
                             const Address&);
      elle::Status      Store(const Network&,
                              const Address&) const;
      elle::Status      Erase(const Network&,
                              const Address&) const;
      elle::Status      Exist(const Network&,
                              const Address&) const;
    };

  }
}

#endif
