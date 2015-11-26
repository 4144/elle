#ifndef INFINIT_REACTOR_SCHEDULER_HH
# define INFINIT_REACTOR_SCHEDULER_HH

# include <memory>
# include <thread>

# include <boost/multi_index_container.hpp>
# include <boost/multi_index/hashed_index.hpp>
# include <boost/multi_index/identity.hpp>
# include <boost/multi_index/sequenced_index.hpp>
# ifdef INFINIT_WINDOWS
#  include <winsock2.h>
# endif
# include <boost/thread.hpp>

# include <elle/Printable.hh>
# include <elle/attribute.hh>

# include <reactor/asio.hh>
# include <reactor/duration.hh>
# include <reactor/fwd.hh>
# include <reactor/backend/fwd.hh>

namespace reactor
{
  /** Scheduler
   *
   */
  class Scheduler:
    public elle::Printable
  {
  /*-------------.
  | Construction |
  `-------------*/
  public:
    Scheduler();
    virtual
    ~Scheduler();

  /*------------------.
  | Current scheduler |
  `------------------*/
  public:
    static
    Scheduler*
    scheduler();

  /*----.
  | Run |
  `----*/
  public:
    void run();
    bool step();
  private:
    virtual
    void
    _rethrow_exception(std::exception_ptr e) const;
    void _step(Thread* t);
    ELLE_ATTRIBUTE_R(bool, done);

  /*-------------------.
  | Threads management |
  `-------------------*/
  public:
    typedef boost::multi_index::multi_index_container<
    Thread*,
    boost::multi_index::indexed_by<
      boost::multi_index::hashed_unique<boost::multi_index::identity<Thread*>>,
      boost::multi_index::sequenced<>
      >
    > Threads;
    Thread* current() const;
    Threads terminate();
    void terminate_now();
    void terminate_later();
  private:
    bool _shallstop;

  private:
    void _freeze(Thread& thread);
    void _thread_register(Thread& thread);
    void _unfreeze(Thread& thread, std::string const& reason);
  private:
    // return true if terminate_now or wait will noop
    bool _terminate(Thread* thread);
    void _terminate_now(Thread* thread,
                        bool suicide);
    Thread* _current;
    Threads _starting;
    boost::mutex _starting_mtx;
    Threads _running;
    Threads _frozen;

    /*-------.
    | Status |
    `-------*/
    public:
      void dump(std::ostream&);
      void debug();

  /*-------------------------.
  | Thread Exception Handler |
  `-------------------------*/
  protected:
    std::exception_ptr _eptr;

  /*----------------.
  | Multithread API |
  `----------------*/
  public:
    template <typename R>
    R
    mt_run(const std::string& name,
           const boost::function<R ()>& action);
  private:
    void
    _mt_run_void(const std::string& name,
                 const boost::function<void ()>& action);

  /*----------.
  | Printable |
  `----------*/
  public:
    void print(std::ostream& s) const;

  /*----------.
  | Shortcuts |
  `----------*/
  public:
    /** Run the given operation in the next cycle.
     *
     *  \param name Descriptive name of the operation, for debugging.
     *  \param f    Operation to run later.
     */
    void run_later(std::string const& name,
                   std::function<void ()> const& f);
    void CallLater(const boost::function<void ()>&      f,
                   const std::string&                   name,
                   Duration                             delay);
    /** Run an operation at a given frequency.
     *
     *  Run the \param op operation every \param freq. For now, the
     *  implementation simply sleeps for \param freq between every action thus
     *  inducing a drift. This is suitable for actions that need to be run
     *  in background roughly regularly such as garbage collecting,
     *  notification checking, ... It is not suitable if the frequency must be
     *  exactly met on the long term.
     *
     *  The thread that run the operation is returned, enabling the caller to
     *  stop it at will.
     *
     *  \param op   The operation to run.
     *  \param name The name of the thread that will run the operation.
     *  \param freq The frequency at which to run the operation.
     *  \param freq Whether to delete the thread when it's done.
     *  \return     The thread running the operation.
     */
    Thread*
    every(const boost::function<void ()>& op,
          const std::string& name,
          Duration freq,
          bool dispose = false);

  /*----------------.
  | Background jobs |
  `----------------*/
  public:
    /// Number of threads spawned to run background jobs.
    int
    background_pool_size() const;
  private:
    template <typename T>
    friend class BackgroundOperation;
    /** Run function in a system thread. Run the result back in the scheduler.
     *
     *  It is guaranteed that a thread will immediately start executing the
     *  action, spawning a new one if needed. The system thread runs freely, all
     *  race condition issues apply, use at your own risks. The thread is joined
     *  upon destruction of the scheduler. The returned function is then run
     *  in the scheduler context.
     *
     *  This pattern enables to run absolutely pure code in the system thread,
     *  removing the possibility of any race condition, and run the
     *  potentially non-pure epilogue in the non-parallel scheduler context.
     *
     *  \param action The action to run in a system thread.
     *  \return A function to run back in the scheduler.
     */
    void
    _run_background(std::function<std::function<void ()> ()> action);
    ELLE_ATTRIBUTE(boost::asio::io_service, background_service);
    ELLE_ATTRIBUTE(boost::asio::io_service::work*, background_service_work);
    ELLE_ATTRIBUTE(std::vector<std::thread>, background_pool);
    ELLE_ATTRIBUTE(int, background_pool_free);
    friend
    void
    background(std::function<void()> const& action);

  /*--------.
  | Signals |
  `--------*/
  public:
    void
    signal_handle(int signal, std::function<void()> const& handler);
  private:
    ELLE_ATTRIBUTE(std::vector<std::unique_ptr<boost::asio::signal_set>>,
                   signal_handlers);

    /*-----.
    | Asio |
    `-----*/
    public:
      boost::asio::io_service& io_service();
    private:
      boost::asio::io_service _io_service;
      boost::asio::io_service::work* _io_service_work;

    /*--------.
    | Details |
    `--------*/
    public:
      backend::Backend& manager();
    private:
      friend class Thread;
      std::unique_ptr<backend::Backend> _manager;
      std::thread::id _running_thread;
  };

  /*---------------.
  | Free functions |
  `---------------*/

  /// The current scheduler.
  reactor::Scheduler&
  scheduler();
  /// Run \a action in a thread and yield until completion.
  void
  background(std::function<void()> const& action);
  /// Yield execution for this scheduler round.
  void
  yield();
  /// Yield for \a duration.
  void
  sleep(Duration duration);
  /// Yield forever.
  void
  sleep();
  /// Wait for \a waitable.
  bool
  wait(Waitable& waitable, DurationOpt timeout = DurationOpt());
  /// Wait for \a waitables.
  bool
  wait(Waitables const& waitables, DurationOpt timeout = DurationOpt());
  /** Run the given operation in the next cycle.
   *
   *  \param name Descriptive name of the operation, for debugging.
   *  \param f    Operation to run later.
   */
  void run_later(std::string const& name,
                 std::function<void ()> const& f);
}

# include <reactor/scheduler.hxx>

#endif
