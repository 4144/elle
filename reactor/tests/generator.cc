#include <elle/log.hh>
#include <elle/test.hh>

#include <reactor/Generator.hh>

ELLE_LOG_COMPONENT("reactor.generator.test");

ELLE_TEST_SCHEDULED(empty)
{
  auto f = [] (reactor::yielder<int>::type const&) {};
  for (int i: reactor::generator<int>(f))
    BOOST_FAIL(elle::sprintf("empty generator yielded a value: %s", i));
}

ELLE_TEST_SCHEDULED(simple)
{
  std::vector<int> results({0, 1, 3});
  auto f = [&] (reactor::yielder<int>::type const& yield)
    {
      for (auto i: results)
        yield(i);
    };
  auto it = begin(results);
  for (int i: reactor::generator<int>(f))
    BOOST_CHECK_EQUAL(i, *(it++));
  BOOST_CHECK(it == results.end());
}

ELLE_TEST_SCHEDULED(move)
{
  auto f = [] (reactor::yielder<std::unique_ptr<int>>::type const& yield)
    {
      yield(elle::make_unique<int>(42));
    };
  bool seen = false;
  for (auto i: reactor::generator<std::unique_ptr<int>>(f))
  {
    BOOST_CHECK(!seen);
    seen = true;
    BOOST_CHECK_EQUAL(*i, 42);
  }
  BOOST_CHECK(seen);
}

ELLE_TEST_SCHEDULED(interleave)
{
  reactor::Barrier sync;
  bool beacon = false;
  auto f = [&] (reactor::yielder<bool>::type const& yield)
    {
      yield(false);
      reactor::wait(sync);
      beacon = true;
      yield(true);
    };
  auto g = reactor::generator<bool>(f);
  auto it = begin(g);
  BOOST_CHECK(it != end(g));
  BOOST_CHECK(!*it);
  BOOST_CHECK(!beacon);
  sync.open();
  ++it;
  BOOST_CHECK(it != end(g));
  BOOST_CHECK(*it);
  ++it;
  BOOST_CHECK(!(it != end(g)));
}

ELLE_TEST_SUITE()
{
  auto& master = boost::unit_test::framework::master_test_suite();
  master.add(BOOST_TEST_CASE(empty));
  master.add(BOOST_TEST_CASE(simple));
  master.add(BOOST_TEST_CASE(move));
  master.add(BOOST_TEST_CASE(interleave));
}
