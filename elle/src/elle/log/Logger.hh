#ifndef ELLE_LOG_LOGGER_HH
# define ELLE_LOG_LOGGER_HH

# include <memory>
# include <mutex>
# include <string>
# include <unordered_map>
# include <vector>

# include <boost/noncopyable.hpp>

# include <elle/attribute.hh>
# include <elle/memory.hh>
# include <elle/time.hh>

namespace elle
{
  namespace log
  {
    namespace detail
    {
      struct Send;
    }

    class Indentation
    {
    public:
      virtual
      ~Indentation();
      virtual
      unsigned int&
      indentation() = 0;
      virtual
      void
      indent() = 0;
      virtual
      void
      unindent() = 0;
    };

    class Indenter
    {
    public:
      typedef std::function<std::unique_ptr<Indentation> ()> Factory;
      virtual
      std::unique_ptr<Indentation>
      indentation(Factory const& factory) = 0;
    };

    class Tag
    {
    public:
      virtual
      std::string
      name() = 0;
      virtual
      std::string
      content() = 0;
    };

    class Logger
      : private boost::noncopyable
    {
    /*------.
    | Level |
    `------*/
    public:
      enum class Level
      {
        none,
        log,
        trace,
        debug,
        dump,
      };

    /*-----.
    | Type |
    `-----*/
    public:
      enum class Type
      {
        info,
        warning,
        error,
      };

    /*-------------.
    | Construction |
    `-------------*/
    public:
      Logger(std::string const& log_level);
      virtual
      ~Logger();

    /*------------.
    | Indentation |
    `------------*/
    private:
      friend struct detail::Send;
      friend
      void
      logger(elle::generic_unique_ptr<Logger> logger);
      unsigned int&
      indentation();
      void
      indent();
      void
      unindent();
      std::recursive_mutex _mutex;
      std::unique_ptr<Indentation> _indentation;
      ELLE_ATTRIBUTE_RW(bool, time_universal);

    /*----------.
    | Messaging |
    `----------*/
    public:
      void message(Level level,
                   elle::log::Logger::Type type,
                   std::string const& component,
                   std::string const& message,
                   std::string const& file,
                   unsigned int line,
                   std::string const& function);
    protected:
      virtual
      void
      _message(Level level,
               elle::log::Logger::Type type,
               std::string const& component,
               boost::posix_time::ptime const& time,
               std::string const& message,
               std::vector<std::pair<std::string, std::string>> const& tags,
               int indentation,
               std::string const& file,
               unsigned int line,
               std::string const& function) = 0;

    /*-----------.
    | Components |
    `-----------*/
    public:
      Level
      component_enabled(std::string const& name);
    private:
      std::vector<std::pair<std::string, Level>> _component_patterns;
      std::unordered_map<std::string, Level> _component_levels;
      ELLE_ATTRIBUTE_R(unsigned int, component_max_size);
    };

    std::ostream&
    operator << (std::ostream& stream, Logger::Level l);
  }
}

# include <elle/log/Logger.hxx>

#endif
