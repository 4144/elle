import drake
import drake.cxx

class GNUBuilder(drake.Builder):

  def __init__(self,
               cxx_toolkit,
               targets = [],
               configure: """Configure script path (or None if no configure
                             step is needed)""" = None,
               configure_args: "Arguments of the configure script" = [],
               sources = [],
               makefile: "Makefile filename, used if not None" = None,
               build_args: "Additional arguments for the make command" = [],
               configure_interpreter = None):
    self.__toolkit = cxx_toolkit
    self.__configure = configure
    self.__configure_interpreter = configure_interpreter
    self.__configure_args = configure_args
    self.__targets = targets
    self.__makefile = makefile
    self.__build_args = build_args
    drake.Builder.__init__(
      self,
      (configure is not None and [configure] or []) + sources,
      targets
    )

  def execute(self):
    # Configure step
    if self.__configure is not None and \
       not self.cmd('Configure %s' % self.work_directory,
                    self.command_configure,
                    cwd = self.work_directory,
                    leave_stdout = True):
      return False

    # Build step
    if not self.cmd('Build %s' % self.work_directory,
                    self.command_build,
                    cwd = self.work_directory,
                    leave_stdout = True):
      return False

    for target in self.__targets:
      path = drake.Path(target.path())
      path.strip_prefix(self.work_directory)
      if not isinstance(target, drake.cxx.DynLib):
        continue
      with drake.WritePermissions(target):
        cmd = self.__toolkit.rpath_set_command(target.path(), '.')
        if not self.cmd('Fix rpath for %s' % target.path(),
                        cmd):
          return False
    return True

  @property
  def command_configure(self):
    basename = str(self.__configure.path().basename())
    if self.__configure_interpreter is None:
      config = ['./%s' % basename]
    else:
      config = [self.__configure_interpreter, basename]
    return config + self.__args

  @property
  def command_build(self):
    if self.__makefile is not None:
      return ['make', '-f', self.__makefile, 'install'] + self.__build_args
    return ['make', 'install'] + self.__build_args

  @property
  def work_directory(self):
    return str(self.__configure.path().dirname())


  def hash(self):
    return str(self.command_configure) + str(self.command_build)
