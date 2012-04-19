#ifndef  NUCLEUS_PROTON_IMMUTABLEBLOCKSERIALIZER_HXX
# define NUCLEUS_PROTON_IMMUTABLEBLOCKSERIALIZER_HXX

# include <nucleus/proton/BlockSerializer.hxx>

# include <nucleus/proton/ImmutableBlock.hh>

ELLE_SERIALIZE_SIMPLE(nucleus::proton::ImmutableBlock,
                      archive,
                      value,
                      version)
{
  assert(version == 0);
  archive & static_cast<nucleus::proton::Block&>(value);
}

#endif


