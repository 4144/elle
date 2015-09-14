#ifndef INFINIT_REACTOR_SEMAPHORE_HH
# define INFINIT_REACTOR_SEMAPHORE_HH

# include <reactor/lockable.hh>

namespace reactor
{
  class Semaphore: public Lockable
  {
    public:
      Semaphore(int count = 0);
      int count() const;
      bool release() override;

    protected:
      virtual bool _wait(Thread* thread) override;

    private:
      int _count;
  };
}

#endif
