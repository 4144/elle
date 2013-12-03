#include "reactor.hh"

#include <elle/test.hh>

#ifdef VALGRIND
# include <valgrind/valgrind.h>
#else
# define RUNNING_ON_VALGRIND 0
#endif

#include <elle/log.hh>
#include <elle/log/TextLogger.hh>
#include <elle/os/setenv.hh>

#include <reactor/thread.hh>

reactor::Scheduler* sched = 0;
elle::log::TextLogger* logger;
std::stringstream ss, res;

static void yield()
{
  reactor::Scheduler::scheduler()->current()->yield();
}

static
void
gen_message(const std::string& thread_name)
{
  ELLE_LOG_COMPONENT("Test");
  ELLE_LOG_SCOPE("Test message from %s", thread_name);
  yield();
  ELLE_LOG("Another message from %s", thread_name);
  yield();
  ELLE_WARN("A third message from %s", thread_name);
}

static
void
scheduler_log_test()
{
  elle::os::setenv("ELLE_LOG_LEVEL", "NONE,Test:DUMP,in:DUMP,out:DUMP", 1);
  logger = new elle::log::TextLogger(ss);
  elle::log::logger(std::unique_ptr<elle::log::Logger>(logger));

  reactor::Scheduler sched;

  reactor::Thread t1(sched, "Thread1",
                     std::bind(gen_message, std::ref("Thread 1")));
  reactor::Thread t2(sched, "Thread2",
                     std::bind(gen_message, std::ref("Thread 2")));

  sched.run();

  res << "[1m[Test] [Thread1] Test message from Thread 1\n[0m"
      << "[1m[Test] [Thread2] Test message from Thread 2\n[0m"
      << "[1m[Test] [Thread1]   Another message from Thread 1\n[0m"
      << "[1m[Test] [Thread2]   Another message from Thread 2\n[0m"
      << "[33;01;33m[Test] [Thread1]   A third message from Thread 1\n[0m"
      << "[33;01;33m[Test] [Thread2]   A third message from Thread 2\n[0m";

  BOOST_CHECK_EQUAL(ss.str(), res.str());
}

static
void
parallel_write()
{
  std::stringstream output;
  std::unique_ptr<elle::log::Logger> logger
    (new elle::log::TextLogger(output));

  logger->component_enabled("in");
  logger->component_enabled("out");
  elle::log::logger(std::move(logger));

  auto action = [](int& counter)
    {
      reactor::Scheduler sched;
      reactor::Thread t(sched, "logger", [&counter]()
        {
          using namespace boost::posix_time;
          ptime deadline =
            microsec_clock::local_time() + seconds(3);
          while (microsec_clock::local_time() < deadline)
          {
            ELLE_LOG_COMPONENT("out");
            ELLE_LOG("out")
            {
              ELLE_LOG_COMPONENT("in");
              ELLE_LOG("in");
            }
            ++counter;
          }
        });
      sched.run();
    };

  std::list<std::thread> threads;
  std::list<int> counters;
  try
  {
    for (int i = 0; i < (RUNNING_ON_VALGRIND ? 4 : 64); ++i)
    {
      counters.push_back(0);
      int& counter = counters.back();
      threads.push_back(std::thread([&](){ action(counter); }));
    }
  }
  catch (...)
  {
    for (auto& thread: threads)
      thread.join();
    throw;
  }

  for (auto& thread: threads)
    thread.join();

  for (auto counter: counters)
  {
    static int const expected = RUNNING_ON_VALGRIND ? 8 : 64;
    BOOST_CHECK_GT(counter, expected);
  }
}

ELLE_TEST_SUITE()
{
  boost::unit_test::test_suite* sched_log = BOOST_TEST_SUITE("Sched Logger");
  boost::unit_test::framework::master_test_suite().add(sched_log);
  sched_log->add(BOOST_TEST_CASE(scheduler_log_test), 0, 10);
  sched_log->add(BOOST_TEST_CASE(parallel_write), 0, 10);
}
