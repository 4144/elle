#include <elle/test.hh>

#include <reactor/backend/backend.hh>
#ifdef INFINIT_WINDOWS
# include <reactor/backend/coro_io/backend.hh>
#else
# include <reactor/backend/boost_context/backend.hh>
#endif

#include <boost/bind.hpp>

using reactor::backend::Thread;

reactor::backend::Backend* m = 0;

static
void
empty()
{}

static
void
one_yield()
{
  m->current()->yield();
}

static
void
inc(int* i)
{
  ++*i;
}

template <typename Backend>
static
void
test_die()
{
  m = new Backend;
  int i = 0;
  {
    auto t = m->make_thread("test_die", boost::bind(inc, &i));
    t->step();
    BOOST_CHECK_EQUAL(i, 1);
    BOOST_CHECK(t->status() == Thread::Status::done);
  }
  {
    auto t = m->make_thread("test_die", boost::bind(inc, &i));
    t->step();
    BOOST_CHECK_EQUAL(i, 2);
    BOOST_CHECK(t->status() == Thread::Status::done);
  }
  delete m;
}

template <typename Backend>
static
void
test_deadlock_creation()
{
  m = new Backend;
  auto t = m->make_thread("test_deadlock_creation", empty);
  t->step();
  BOOST_CHECK(t->status() == Thread::Status::done);
  delete m;
}

template <typename Backend>
static
void
test_deadlock_switch()
{
  m = new Backend;
  auto t = m->make_thread("test_deadlock_switch", one_yield);
  t->step();
  t->step();
  BOOST_CHECK(t->status() == Thread::Status::done);
  delete m;
}

static
void
status_coro()
{
  BOOST_CHECK(m->current()->status() == Thread::Status::running);
  m->current()->yield();
  BOOST_CHECK(m->current()->status() == Thread::Status::running);
}

template <typename Backend>
static
void
test_status()
{
  m = new Backend;
  auto t = m->make_thread("status", &status_coro);
  BOOST_CHECK(t->status() == Thread::Status::starting);
  t->step();
  BOOST_CHECK(t->status() == Thread::Status::waiting);
  t->step();
  BOOST_CHECK(t->status() == Thread::Status::done);
  delete m;
}

ELLE_TEST_SUITE()
{
  boost::unit_test::test_suite* backend = BOOST_TEST_SUITE("Backend");
  boost::unit_test::framework::master_test_suite().add(backend);
#ifdef INFINIT_WINDOWS
# define TEST(Name)                                                     \
  {                                                                     \
    backend->add(                                                       \
      BOOST_TEST_CASE(Name<reactor::backend::coro_io::Backend>),        \
      0, 10);                                                           \
  }
#else
# define TEST(Name)                                                     \
  {                                                                     \
    backend->add(                                                       \
      BOOST_TEST_CASE(Name<reactor::backend::boost_context::Backend>),  \
      0, 10);                                                           \
  }
#endif
  TEST(test_die);
  TEST(test_deadlock_creation);
  TEST(test_deadlock_switch);
  TEST(test_status);
}
