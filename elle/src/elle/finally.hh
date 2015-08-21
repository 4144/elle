#ifndef ELLE_FINALLY_HH
# define ELLE_FINALLY_HH

# include <functional>
# include <cstdlib>

# include <boost/preprocessor/cat.hpp>

# include <elle/With.hh>
# include <elle/attribute.hh>

namespace elle
{
  class SafeFinally
  {
    typedef std::function<void()> Action;
  /*-------------.
  | Construction |
  `-------------*/
  public:
    SafeFinally();
    SafeFinally(Action const& action);
    ~SafeFinally();
    SafeFinally(SafeFinally&& f) = delete;
    SafeFinally(SafeFinally const& f) = delete;

  /*--------.
  | Methods |
  `--------*/
  public:
    void
    abort();

    bool
    aborted() const;
  /*-----------.
  | Attributes |
  `-----------*/
  private:
    ELLE_ATTRIBUTE_RW(Action, action);
  };

  /// Provide a way for a final action to be performed i.e when leaving the
  /// current scope.
  ///
  /// Note that a method is provided (i.e abort()) for cancelling this final
  /// action.
  class Finally
  {
  /*-------------.
  | Construction |
  `-------------*/
  protected:
    Finally();
    Finally(std::function<void()> const& action);
    ~Finally() noexcept(false);
    /// Let With manage us.
    template<typename> friend class elle::With;

  /*--------.
  | Methods |
  `--------*/
  public:
    void
    abort();

  /*-----------.
  | Attributes |
  `-----------*/
  private:
    ELLE_ATTRIBUTE_RW(std::function<void()>, action);
  };
}

#endif
