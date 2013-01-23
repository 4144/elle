#ifndef  NUCLEUS_DERIVABLE_HXX
# define NUCLEUS_DERIVABLE_HXX

# include <stdexcept>

# include <elle/serialize/Serializer.hh>

# include <nucleus/factory.hh>

ELLE_SERIALIZE_NO_FORMAT(nucleus::Derivable);

ELLE_SERIALIZE_SPLIT(nucleus::Derivable);

ELLE_SERIALIZE_SPLIT_LOAD(nucleus::Derivable, archive, value, version)
{
  enforce(value.kind == nucleus::Derivable::Kind::output);
  enforce(version == 0);
  archive >> value._component;

  if (value._dynamic_construct)
    {
      enforce(value._block == nullptr);

      value._block =
        nucleus::factory::block().allocate<nucleus::proton::Block>(
          value._component);
    }
  enforce(value._block != nullptr);
  typedef typename elle::serialize::SerializableFor<Archive>::Type interface_t;
  enforce(dynamic_cast<interface_t*>(value._block) != nullptr);
  static_cast<interface_t&>(*value._block).deserialize(archive);
}

ELLE_SERIALIZE_SPLIT_SAVE(nucleus::Derivable, archive, value, version)
{
  enforce(version == 0);
  enforce(value._block != nullptr);
  archive << value._component;
  archive << *value._block;
}

#endif
