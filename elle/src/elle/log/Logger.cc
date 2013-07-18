#include <fnmatch.h>

#include <boost/algorithm/string.hpp>
#include <boost/thread/tss.hpp>

#include <elle/Exception.hh>
#include <elle/log/Logger.hh>
#include <elle/os/getenv.hh>
#include <elle/printf.hh>


namespace elle
{
  namespace log
  {
    /*------------.
    | Indentation |
    `------------*/

    class PlainIndentation: public Indentation
    {
    public:
      PlainIndentation()
        : _indentation()
      {}

      virtual
      unsigned int&
      indentation()
      {
        if (!this->_indentation.get())
          this->_indentation.reset(new unsigned int(0));
        return *this->_indentation;
      }

      virtual
      void
      indent()
      {
        this->indentation() += 1;
      }

      virtual
      void
      unindent()
      {
        assert(this->indentation() >= 1);
        this->indentation() -= 1;
      }

    private:
      boost::thread_specific_ptr<unsigned int> _indentation;
    };

    std::function<std::unique_ptr<Indentation> ()>&
    Logger::_factory()
    {
      static std::function<std::unique_ptr<Indentation> ()> res =
        [] () { return std::unique_ptr<Indentation>(new PlainIndentation()); };
      return res;
    }

    unsigned int&
    Logger::indentation()
    {
      return this->_indentation->indentation();
    }

    void
    Logger::indent()
    {
      this->_indentation->indent();
    }

    void
    Logger::unindent()
    {
      this->_indentation->unindent();
    }

    /*----.
    | Tag |
    `----*/

    std::vector<std::unique_ptr<Tag>>&
    Logger::_tags()
    {
      static std::vector<std::unique_ptr<Tag>> res;
      return res;
    }

    /*-------------.
    | Construction |
    `-------------*/

    static
    Logger::Level
    parse_level(std::string const& level)
    {
      if (level == "NONE")
        return Logger::Level::none;
      else if (level == "LOG")
        return Logger::Level::log;
      else if (level == "TRACE")
        return Logger::Level::trace;
      else if (level == "DEBUG")
        return Logger::Level::debug;
      else if (level == "DUMP")
        return Logger::Level::dump;
      else
        throw elle::Exception(
          elle::sprintf("invalid log level: %s", level));
  }

    Logger::Logger()
      : _indentation(this->_factory()())
      , _component_patterns()
      , _component_levels()
      , _component_max_size(0)
    {
      std::string levels_str = elle::os::getenv("ELLE_LOG_LEVEL", "");
      if (!levels_str.empty())
      {
        std::vector<std::string> levels;
        boost::algorithm::split(levels, levels_str,
                                boost::algorithm::is_any_of(","),
                                boost::algorithm::token_compress_on);
        for (auto& pattern: levels)
        {
          auto colon = pattern.find(":");
          Level level = Level::log;
          if (colon != std::string::npos)
          {
            std::string level_str = pattern.substr(colon + 1);
            boost::algorithm::trim(level_str);
            level = parse_level(level_str);
            pattern = pattern.substr(0, colon);
            boost::algorithm::trim(pattern);
          }
          else
          {
            boost::algorithm::trim(pattern);
            level = parse_level(pattern);
            pattern = "*";
          }
          _component_patterns.push_back(std::make_pair(pattern, level));
        }
      }
    }

    Logger::~Logger()
    {}

    /*----------.
    | Messaging |
    `----------*/

    void
    Logger::message(Level level,
                    elle::log::Logger::Type type,
                    std::string const& component,
                    std::string const& msg,
                    std::string const& file,
                    unsigned int line,
                    std::string const& function)
    {
      std::lock_guard<std::recursive_mutex> lock(_mutex);

      int indent = this->indentation();
      ELLE_ASSERT_GTE(indent, 1);

      std::string message = msg;
      for (auto& tag: this->_tags())
        {
          std::string content(tag->content());
          if (!content.empty())
            message = elle::sprintf("[%s] ", content) + message;
        }
      this->_message(level, type, component, message,
                     indent - 1, file, line, function);
    }

    /*--------.
    | Enabled |
    `--------*/

    Logger::Level
    Logger::component_enabled(std::string const& name)
    {
      std::lock_guard<std::recursive_mutex> lock(_mutex);
      auto elt = this->_component_levels.find(name);
      Level res = Level::log;
      if (elt == this->_component_levels.end())
      {
        for (auto const& pattern: this->_component_patterns)
          if (fnmatch(pattern.first.c_str(), name.c_str(), 0) == 0)
            res = pattern.second;
        if (res > Level::none)
          this->_component_max_size =
            std::max(this->_component_max_size,
                     static_cast<unsigned int>(name.size()));
        this->_component_levels[name] = res;
      }
      else
        res = elt->second;
      return res;
    }

    /*------.
    | Level |
    `------*/

    std::ostream&
    operator << (std::ostream& stream, Logger::Level l)
    {
      switch (l)
      {
        case Logger::Level::none:
          stream << "none";
          break;
        case Logger::Level::log:
          stream << "log";
          break;
        case Logger::Level::trace:
          stream << "trace";
          break;
        case Logger::Level::debug:
          stream << "debug";
          break;
        case Logger::Level::dump:
          stream << "dump";
          break;
      }
      return stream;
    }
  }
}
