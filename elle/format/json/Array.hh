#ifndef ELLE_FORMAT_JSON_ARRAY_HH
# define ELLE_FORMAT_JSON_ARRAY_HH

# include <cassert>
# include <vector>

# include "Object.hh"

namespace elle { namespace format { namespace json {

    class Null;

    class Array : public Object
    {
    private:
      std::vector<Object*> _value;

    public:
      Array() {}

      template<typename Container>
      Array(Container const& container);

      ~Array()
      {
        for (auto it = _value.begin(), end = _value.end(); it != end; ++it)
          delete (*it);
        _value.clear();
      }
      template<typename T> inline void push_back(T const& value);
      inline void push_back(std::unique_ptr<Object>&& value)
        {
          assert(value.get() != nullptr);
          _value.push_back(value.get());
          value.release();
        }

      size_t size() const { return _value.size(); }

      std::unique_ptr<Object> Clone() const;
    protected:
      void Save(elle::serialize::OutputJSONArchive& ar) const;

    public:
      using Object::operator ==;

      virtual bool operator ==(Object const& other) const
        { return other == *this; }

      virtual bool operator ==(Array const& other) const
        {
          if (this->size() != other.size())
            return false;
          for (size_t i = 0; i < this->size(); ++i)
            if (*_value[i] != *other._value[i])
              return false;
          return true;
        }
    };

}}} // !namespace elle::format::json

#endif /* ! ARRAY_HH */


