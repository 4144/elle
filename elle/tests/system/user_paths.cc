#include <elle/os/path.hh>
#include <elle/system/user_paths.hh>
#include <elle/test.hh>

static
void
home_directory()
{
  auto const& home_dir = elle::system::home_directory().string();
  BOOST_CHECK(elle::os::path::exists(home_dir));
  BOOST_CHECK(elle::os::path::is_directory(home_dir));
}

static
void
download_directory()
{
  std::string const& download_dir = elle::system::download_directory().string();
  BOOST_CHECK(elle::os::path::exists(download_dir));
  BOOST_CHECK(elle::os::path::is_directory(download_dir));
#ifdef INFINIT_MACOSX
  // Download folder on OS X is language dependent.
#else
  // Some machines may not have a download directory in which case
  // elle::system::download_directory falls back to the home directory.
  if (download_dir != elle::system::home_directory())
    BOOST_CHECK(download_dir.find("Downloads") != std::npos);
#endif
}

ELLE_TEST_SUITE()
{
  auto& suite = boost::unit_test::framework::master_test_suite();
  suite.add(BOOST_TEST_CASE(home_directory));
  suite.add(BOOST_TEST_CASE(download_directory));
}
