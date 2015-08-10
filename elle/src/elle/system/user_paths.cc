#include <iostream>

#include <elle/os/environ.hh>
#include <elle/os/path.hh>
#include <elle/system/user_paths.hh>
#ifdef INFINIT_WINDOWS
# include <elle/windows/string_conversion.hh>
# include <shlobj.h>
#else
# include <pwd.h>
#endif

namespace elle
{
  namespace system
  {
    boost::filesystem::path
    home_directory()
    {
#if defined(INFINIT_IOS) || defined(INFINIT_MACOSX)
# error Use user_paths.mm on Apple platforms.
#elif defined(INFINIT_WINDOWS)
      wchar_t path[MAX_PATH];
      if (SUCCEEDED(SHGetSpecialFolderPathW(NULL, path, CSIDL_PROFILE, 0)))
      {
        wchar_t path2[MAX_PATH];
        auto len = GetShortPathNameW(path, NULL, 0);
        if (GetShortPathNameW (path, path2, len) != 0)
        {
          return {elle::string::wstring_to_string(path2)};
        }
      }

      // Backup solution.
      return (
        boost::filesystem::path{os::getenv("SystemDrive", "C:")} /
        boost::filesystem::path{os::getenv("HOMEPATH", "")}
        );
#else
      struct passwd* pw = ::getpwuid(getuid());
      if (pw != nullptr && pw->pw_dir != nullptr)
        return {pw->pw_dir};
      else
        return {os::getenv("HOME", "/tmp")};
#endif
    }

    boost::filesystem::path
    download_directory()
    {
      auto candiate_dir = elle::path::join(home_directory(), "Downloads");
      using path = elle::os::path;
      if (path::exists(candiate_dir) && path::is_directory(candiate_dir))
        return candiate_dir;
      return home_directory();
    }
  }
}
