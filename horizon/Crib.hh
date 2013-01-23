#ifndef HORIZON_CRIB_HH
# define HORIZON_CRIB_HH

# include <elle/types.hh>

# include <horizon/Handle.hh>

# include <elle/idiom/Close.hh>
#  include <map>
# include <elle/idiom/Open.hh>

namespace horizon
{
  /// Relations between created files' way and handle.
  class Crib
  {
  public:
    //
    // types
    //
    typedef std::map<const elle::String, Handle*>     Container;
    typedef Container::iterator                       Iterator;

    //
    // static methods
    //
    static elle::Status       Add(const elle::String&,
                                  Handle*);
    static elle::Boolean      Exist(const elle::String&);
    static elle::Status       Retrieve(const elle::String&,
                                       Handle*&);
    static elle::Status       Remove(const elle::String&);

    static elle::Status       Show(const elle::Natural32 = 0);
    /// XXX
    static
    void
    rename(const char* source,
           const char* target);

    //
    // static attributes
    //
    static Container          Handles;
  };
}

#endif
