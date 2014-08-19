#ifndef INFINIT_REACTOR_BACKEND_CORO_IO_THREAD_HH
# define INFINIT_REACTOR_BACKEND_CORO_IO_THREAD_HH

# include <string>

# include <reactor/backend/backend.hh>

struct Coro;

namespace reactor
{
  namespace backend
  {
    namespace boost_context
    {
      class Backend:
        public reactor::backend::Backend
      {
      /*------.
      | Types |
      `------*/
      public:
        typedef Backend Self;
        typedef reactor::backend::Backend Super;
        class Thread;

      /*-------------.
      | Construction |
      `-------------*/
      public:
        Backend();
        ~Backend();

      /*--------.
      | Threads |
      `--------*/
      public:
        virtual
        std::unique_ptr<backend::Thread>
        make_thread(const std::string& name,
                    const Action& action) override;
        virtual
        backend::Thread*
        current() const override;

      /*--------.
      | Details |
      `--------*/
      private:
        /// Let threads manipulate the current thread and the root thread.
        friend class Thread;
        /// Root thread, which instantiated the Backend.
        std::unique_ptr<Thread> _self;
        /// Current thread.
        Thread* _current;
      };
    }
  }
}

#endif
