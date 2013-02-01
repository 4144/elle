#ifndef HORIZON_CRUX_HH
# define HORIZON_CRUX_HH

# ifndef FUSE_USE_VERSION
#  define FUSE_USE_VERSION               26
# endif

# include <elle/types.hh>
# include <elle/system/platform.hh>

# include <nucleus/neutron/Size.hh>

# include <fuse.h>
# if defined(HAVE_SETXATTR)
#   if defined(INFINIT_LINUX)
#    include <attr/xattr.h>
#   elif defined(INFINIT_MACOSX)
#    include <sys/xattr.h>
#   endif
# endif

namespace horizon
{
  /// Implementation of the FUSE upcalls.
  ///
  /// XXX root of any calls i.e crux
  class Crux
  {
  public:
    // constants
    static const nucleus::neutron::Size Range;

  /*---------.
  | Metadata |
  `---------*/
  public:
    static
    int
    getattr(const char*,
            struct ::stat*);
    static
    int
    fgetattr(const char*,
             struct ::stat*,
             struct ::fuse_file_info*);
    static
    int
    utimens(const char*,
            const struct ::timespec[2]);

  /*----------.
  | Directory |
  `----------*/
  public:
    static
    int
    opendir(const char*,
            struct ::fuse_file_info*);
    static
    int
    readdir(const char*,
            void*,
            ::fuse_fill_dir_t,
            off_t,
            struct ::fuse_file_info*);
    static
    int
    releasedir(const char*,
               struct ::fuse_file_info*);
    static
    int
    mkdir(const char*,
          mode_t);
    static
    int
    rmdir(const char*);

  /*------------.
  | Permissions |
  `------------*/
  public:
    static
    int
    access(const char*,
           int);
    static
    int
    chmod(const char*,
          mode_t);
    static
    int
    chown(const char*,
          uid_t,
          gid_t);

# if defined(HAVE_SETXATTR)
    /*--------------------.
    | Extended attributes |
    `--------------------*/
  public:
    static
    int
    setxattr(const char*,
             const char*,
             const char*,
             size_t,
             int);
    static
    int
    getxattr(const char*,
             const char*,
             char*,
             size_t);
    static
    int
    listxattr(const char*,
              char*,
              size_t);
    static
    int
    removexattr(const char*,
                const char*);
# endif

  /*-----.
  | Link |
  `-----*/
  public:
    static
    int
    link(const char* target,
         const char* source);
    static
    int
    symlink(const char*,
            const char*);
    static
    int
    readlink(const char*,
             char*,
             size_t);

  /*-----.
  | File |
  `-----*/
  public:
    static
    int
    create(const char*,
           mode_t,
           struct ::fuse_file_info*);
    static
    int
    open(const char*,
         struct ::fuse_file_info*);
    static
    int
    write(const char*,
          const char*,
          size_t,
          off_t,
          struct ::fuse_file_info*);
    static
    int
    read(const char*,
         char*,
         size_t,
         off_t,
         struct ::fuse_file_info*);
    static
    int
    truncate(const char*,
             off_t);
    static
    int
    ftruncate(const char*,
              off_t,
              struct ::fuse_file_info*);
    static
    int
    release(const char*,
            struct ::fuse_file_info*);

  /*--------.
  | Objects |
  `--------*/
  public:
    static
    int
    rename(const char*,
           const char*);
    static
    int
    unlink(const char*);
  };
}

#endif
