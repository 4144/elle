#ifndef ELLE_SERIALIZATION_BINARY_SERIALIZERIN_HH
# define ELLE_SERIALIZATION_BINARY_SERIALIZERIN_HH

# include <vector>

# include <elle/attribute.hh>
# include <elle/serialization/SerializerIn.hh>

namespace elle
{
  namespace serialization
  {
    namespace binary
    {
       class SerializerIn:
        public serialization::SerializerIn
      {
      /*------.
      | Types |
      `------*/
      public:
        typedef SerializerIn Self;
        typedef serialization::SerializerIn Super;

      /*-------------.
      | Construction |
      `-------------*/
      public:
        SerializerIn(std::istream& input, bool versioned = true);
        SerializerIn(std::istream& input, Versions versions);
      private:
        void
        _check_magic(std::istream& input);

      /*--------------.
      | Serialization |
      `--------------*/
      protected:
        virtual
        bool
        _text() const override;
        virtual
        void
        _serialize(std::string const& name, int64_t& v) override;
        virtual
        void
        _serialize(std::string const& name, uint64_t& v) override;
        virtual
        void
        _serialize(std::string const& name, int32_t& v) override;
        virtual
        void
        _serialize(std::string const& name, uint32_t& v) override;
        virtual
        void
        _serialize(std::string const& name, int16_t& v) override;
        virtual
        void
        _serialize(std::string const& name, uint16_t& v) override;
        virtual
        void
        _serialize(std::string const& name, int8_t& v) override;
        virtual
        void
        _serialize(std::string const& name, uint8_t& v) override;
        virtual
        void
        _serialize(std::string const& name, double& v) override;
        virtual
        void
        _serialize(std::string const& name, bool& v) override;
        virtual
        void
        _serialize(std::string const& name, std::string& v) override;
        virtual
        void
        _serialize(std::string const& name, elle::Buffer& v) override;
        virtual
        void
        _serialize(std::string const& name,
                   boost::posix_time::ptime& v) override;
        virtual
        void
        _serialize_named_option(std::string const& name,
                          bool,
                          std::function<void ()> const& f) override;
        virtual
        void
        _serialize_option(std::string const& name,
                          bool,
                          std::function<void ()> const& f) override;
        virtual
        void
        _serialize_array(std::string const& name,
                         int size,
                         std::function<void ()> const& f) override;
        virtual
        void
        _deserialize_dict_key(
          std::function<void (std::string const&)> const& f) override;

        virtual
        bool
        _enter(std::string const& name) override;
        virtual
        void
        _leave(std::string const& name) override;
      private:
        int64_t _serialize_number();
        template <typename T>
        void
        _serialize_int(std::string const& name, T& v);
    };
  }
}

}

#endif
