#ifndef ELLE_CONCURRENCY_PROGRAM_HH
# define ELLE_CONCURRENCY_PROGRAM_HH

# include <elle/types.hh>
# include <elle/CrashReporter.hh>

# include <elle/idiom/Close.hh>
#  include <reactor/signal.hh>
# include <elle/idiom/Open.hh>

// XXX
namespace reactor
{
  class Scheduler;
}

namespace elle
{
  namespace concurrency
  {

    ///
    /// this class represents the running program.
    ///
    class Program
    {
    public:
      //XXX: Static SCOPPED guard suxx a bit.
      static signal::ScoppedGuard _signal_guard;

    public:
      Program();
      //
      // static methods
      //
      static Status             Setup();
      static void               Launch();
      static Void               Exception(int);
      static void               Exit();

      //
      // attributes
      //
    private:
      static reactor::Signal    _exit;
    };

  }
}

#endif
