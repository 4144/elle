#ifndef ELLE_OPTION_HH
# define ELLE_OPTION_HH

# include <cstddef>
# include <type_traits>
# include <iosfwd>

# include <elle/serialization/Serializer.hh>

namespace elle
{
  namespace _details
  {
    template <std::size_t Size, int Index, typename ... Types>
    class OptionHelper;
  }

  template <typename ... Types>
  class Option
    : public _details::OptionHelper<0u, 0, Types ...>
  {
  public:
    typedef Option<Types ...> Self;
    typedef _details::OptionHelper<0u, 0, Types ...> Super;
    using Super::Super;
    template <typename T>
    Option(T&& value);
    Option(Self const& source);
    Option(Self& source);
    Option(Self&& source);
    Self&
    operator =(Self&& option);
    Self&
    operator =(Self const& option);

    template <typename T>
    T&
    get();
    template <typename T>
    T const&
    get() const;
    template <typename T>
    bool
    is() const;
    template <typename T>
    void
    emplace(T&& value);
    template <typename T, typename ... Args>
    void
    emplace(Args&& ... value);

  /*--------------.
  | Serialization |
  `--------------*/
  public:
    typedef elle::serialization_tag serialization_tag;
    Option(serialization::SerializerIn& s);
    void
    serialize(serialization::Serializer& s);

  /*----------.
  | Printable |
  `----------*/
  private:
    template <typename ... Args>
    friend
    std::ostream&
    operator << (std::ostream& output, Option<Args...> const& option);
  };
}

# include <elle/Option.hxx>

#endif
