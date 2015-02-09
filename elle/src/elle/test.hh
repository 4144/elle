#ifndef ELLE_TEST_HH
# define ELLE_TEST_HH

# ifdef VALGRIND
#  include <valgrind/valgrind.h>
# else
#  define RUNNING_ON_VALGRIND 0
# endif

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>

#include <elle/log.hh>

/// This header includes boost Unit Test Framework and provides a simple macro
/// to customize the creation of your test suite.
///
/// There are two supported ways to define your tests:
/// 1. Using BOOST_AUTO_TEST_CASE and generated test suite:
/// {{{
///     #define ELLE_TEST_MODULE "My module"
///     #include <elle/test.hh>
///     BOOST_AUTO_TEST_CASE(SimpleCase) { /* Test your code here */ }
///.}}}
///
/// 2. By Defining your own test suite:
/// {{{
///     #include <elle/test.hh>
///     ELLE_TEST_SUITE() { /* Create and initialize your test suite here */ }
/// }}}
///

# include <elle/Exception.hh>

# ifdef ELLE_TEST_MODULE
#  define BOOST_TEST_MODULE ELLE_TEST_MODULE
# endif

# ifdef INFINIT_WINDOWS
#  include <winsock2.h>
# endif
# include <boost/test/unit_test.hpp>

static std::string test_binary;

# if defined(BOOST_TEST_DYN_LINK)

#  define ELLE_TEST_SUITE()                             \
static                                                  \
void                                                    \
_test_suite();                                          \
                                                        \
bool                                                    \
init_unit_test_suite();                                 \
bool                                                    \
init_unit_test_suite()                                  \
{                                                       \
  try                                                   \
  {                                                     \
    _test_suite();                                      \
  }                                                     \
  catch (...)                                           \
  {                                                     \
    throw boost::unit_test::framework::setup_error      \
      (elle::exception_string());                       \
  }                                                     \
  return true;                                          \
}                                                       \
                                                        \
int                                                     \
main(int ac, char** av)                                 \
{                                                       \
  test_binary = av[0];                                  \
  return ::boost::unit_test::unit_test_main(            \
    &init_unit_test_suite, ac, av);                     \
}                                                       \
                                                        \
                                                        \
static                                                  \
void                                                    \
_test_suite()

# elif defined(BOOST_TEST_STATIC_LINK)

#  define ELLE_TEST_SUITE()                             \
static                                                  \
void                                                    \
_test_suite();                                          \
                                                        \
boost::unit_test::test_suite*                           \
init_unit_test_suite(int, char**);                      \
boost::unit_test::test_suite*                           \
init_unit_test_suite(int ac, char** av)                 \
{                                                       \
  test_binary = av[0];                                  \
  try                                                   \
  {                                                     \
    _test_suite();                                      \
  }                                                     \
  catch (...)                                           \
  {                                                     \
    throw boost::unit_test::framework::setup_error      \
      (elle::exception_string());                       \
  }                                                     \
  return nullptr;                                       \
}                                                       \
                                                        \
static                                                  \
void                                                    \
_test_suite()                                           \

# else
#  error "please define BOOST_TEST_DYN_LINK or BOOST_TEST_STATIC_LINK"
# endif

# define ELLE_TEST_PROTOTYPE_HELPER(R, Data, I, Elem)                   \
  BOOST_PP_COMMA_IF(I)                                                  \
  BOOST_PP_TUPLE_ELEM(0, Elem) BOOST_PP_TUPLE_ELEM(1, Elem)             \

# define ELLE_TEST_PROTOTYPE(Args)                                      \
  BOOST_PP_SEQ_FOR_EACH_I(ELLE_TEST_PROTOTYPE_HELPER, _, Args)          \

# define ELLE_TEST_CALL_HELPER(R, Data, I, Elem)                        \
  BOOST_PP_COMMA_IF(I)                                                  \
  BOOST_PP_TUPLE_ELEM(1, Elem)                                          \

# define ELLE_TEST_CALL(Args)                                           \
  BOOST_PP_SEQ_FOR_EACH_I(ELLE_TEST_CALL_HELPER, _, Args)               \

# define ELLE_TEST_SCHEDULED(...)                                       \
  ELLE_TEST_SCHEDULED_SEQ(BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))        \

# define ELLE_TEST_SCHEDULED_SEQ(Seq)                                   \
  ELLE_TEST_SCHEDULED_HELPER(BOOST_PP_SEQ_HEAD(Seq),                    \
                             BOOST_PP_SEQ_TAIL(Seq))                    \

# ifdef INFINIT_WINDOWS
#  define ELLE_TEST_HANDLE_SIGALRM(Sched)
# else
#  define ELLE_TEST_HANDLE_SIGALRM(Sched)                             \
  Sched.signal_handle(SIGALRM,                                        \
                      []                                              \
                      {                                               \
                        ELLE_ERR("test timeout: SIGALRM");            \
                        throw elle::Error("test timeout");            \
                      });
# endif


# define ELLE_TEST_SCHEDULED_HELPER(Name, Args)                       \
static                                                                \
void                                                                  \
BOOST_PP_CAT(Name,_impl)(ELLE_TEST_PROTOTYPE(Args));                  \
                                                                      \
static                                                                \
void                                                                  \
Name(ELLE_TEST_PROTOTYPE(Args))                                       \
{                                                                     \
  reactor::Scheduler sched;                                           \
  ELLE_TEST_HANDLE_SIGALRM(sched);                                    \
  reactor::Thread main(                                               \
    sched, "main",                                                    \
    [&]                                                               \
    {                                                                 \
      ELLE_LOG_COMPONENT("elle.Test")                                 \
      ELLE_LOG("starting test: %s", BOOST_PP_STRINGIZE(Name));        \
      BOOST_PP_CAT(Name,_impl)(ELLE_TEST_CALL(Args));                 \
    });                                                               \
  sched.run();                                                        \
}                                                                     \
                                                                      \
static                                                                \
void                                                                  \
BOOST_PP_CAT(Name, _impl)(ELLE_TEST_PROTOTYPE(Args))                  \

#define ELLE_TEST_SCHEDULED_THROWS(Name, _exception_type_)            \
static                                                                \
void                                                                  \
Name##_impl();                                                        \
                                                                      \
static                                                                \
void                                                                  \
Name()                                                                \
{                                                                     \
  reactor::Scheduler sched;                                           \
  reactor::Thread main(                                               \
    sched, "main",                                                    \
    [&]                                                               \
    {                                                                 \
      ELLE_LOG_COMPONENT("elle.Test")                                 \
      ELLE_LOG("starting test: %s", BOOST_PP_STRINGIZE(Name));        \
      Name##_impl();                                                  \
    });                                                               \
  BOOST_CHECK_THROW(sched.run(), _exception_type_);                   \
}                                                                     \
                                                                      \
static                                                                \
void                                                                  \
Name##_impl()                                                         \

# ifndef ELLE_TEST_NO_MEMFRY

# include <iostream>
void*
operator new(std::size_t n, std::nothrow_t const&) throw()
{
  if (RUNNING_ON_VALGRIND)
    return ::malloc(n);
  else
  {
    char* chunk = reinterpret_cast<char*>(::malloc(n + sizeof(std::size_t)));
    *reinterpret_cast<std::size_t*>(chunk) = n;
    ::memset(chunk + sizeof(std::size_t), 0xd0, n);
    void* res = chunk + sizeof(std::size_t);
    return res;
  }
}

void*
operator new(std::size_t n) throw(std::bad_alloc)
{
  void* res = ::operator new(n, std::nothrow);
  if (!res)
    throw std::bad_alloc();
  return res;
}

void
operator delete(void* p) throw()
{
  if (!p)
    return;
  if (RUNNING_ON_VALGRIND)
    ::free(p);
  else
  {
    char* chunk = reinterpret_cast<char*>(p) - sizeof(std::size_t);
    std::size_t n = *(reinterpret_cast<std::size_t*>(chunk));
    ::memset(chunk, 0xdf, n + sizeof(std::size_t));
    ::free(chunk);
  }
}
# endif

# ifdef __arm__
#  define ARM_FACTOR 1
# else
#  define ARM_FACTOR 0
# endif

template <typename T>
static
auto
valgrind(T base, int factor = 50) -> decltype(base * 42)
{
  return base * (RUNNING_ON_VALGRIND ? factor : 1) * (ARM_FACTOR ? factor : 1);
}

#endif
