#ifndef REACTOR_SCOPE_HH
# define REACTOR_SCOPE_HH

# include <reactor/Barrier.hh>
# include <reactor/fwd.hh>
# include <reactor/thread.hh>

namespace reactor
{
  /// Manage a collection of Threads.
  ///
  /// A Scope enables to start parallel tasks in threads and make sure they are
  /// terminated upon destruction.  As a Waitable, a Scope blocks its waiters
  /// until all currently managed threads are done, enabling to easily join the
  /// group of managed threads.  If an exception escapes from a thread, all
  /// other threads are killed and waiting the Scope will re-throw the
  /// exception.
  class Scope:
    public reactor::Waitable
  {
  /*-------------.
  | Construction |
  `-------------*/
  public:
    /// Create a scope.
    Scope();
    /// Destruct a scope, terminating all managed threads now.
    ///
    /// \throw Terminate if interrupted while killing managed threads.
    ~Scope() noexcept(false);

  /*--------.
  | Threads |
  `--------*/
  public:
    /// Start and manage a thread named \a name executing \a a.
    ///
    /// If an exception escapes \a a, all other threads are killed and waiting
    /// the Scope re-throws the exception.
    ///
    /// \param name The name of the managed thread.
    /// \param a    The action run by the managed thread.
    void
    run_background(std::string const& name,
                   Thread::Action const& a);
  private:
    void
    _terminate_now();
    /// Managed threads.
    ELLE_ATTRIBUTE(std::vector<Thread*>, threads);
    /// Number of running threads.
    ELLE_ATTRIBUTE(int, running);

  /*---------.
  | Waitable |
  `---------*/
  protected:
    bool
    _wait(Thread* thread);
  private:
    ELLE_ATTRIBUTE(reactor::Barrier, barrier);
  };
}


#endif
