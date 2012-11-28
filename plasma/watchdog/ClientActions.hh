#ifndef  PLASMA_WATCHDOG_CLIENTACTIONS_HH
# define PLASMA_WATCHDOG_CLIENTACTIONS_HH

# include <elle/format/json.hh>

namespace plasma
{
  namespace watchdog
  {
    namespace json = elle::format::json;

    class Manager;
    class Client;
    class Connection;

    ///
    /// Store all local client commands. This class acts as a god interface
    /// between components.
    ///
    /// Each command is encoded in JSon format as a dictionary. It has to
    /// contain a "command" key, that contains the name of the command and
    /// the key "_id" which is watchdog id. Any other key/value pair is allowed
    /// and may represent the command arguments.
    ///
    /// The response is also a JSon dictionary that contains a "success" key,
    /// and when its value (a boolean) is false, then the response also
    /// contains the "error" key, which is a usefull descriptive string ;)
    ///
    /// Commands are described below with their own callback.
    ///
    class ClientActions
    {
    private:
      Manager&          _manager;
      std::string       _watchdog_id;
      std::string const _user_id;

    public:
      ClientActions(Manager& manager,
                    std::string const& user_id);
      ~ClientActions();

    // properties
    public:
      std::string const& watchdog_id() const { return this->_watchdog_id; }
      void watchdog_id(std::string const& id) { this->_watchdog_id = id; }

    private:
      ///
      /// Run command is fired by the updater, it gives the token.
      ///   Q: {
      ///       "command": "run",
      ///       "_id": "watchdog id",
      ///       "token": "meta token",
      ///   }
      ///
      ///   R: no response
      ///
      void _on_run(Connection& conn, Client& client, json::Dictionary const& args);


      ///
      /// Stop completely the watchdog.
      ///   Q: {
      ///       "command": "stop",
      ///       "_id": "watchdog id",
      ///   }
      ///
      ///   R: no response
      ///
      void _on_stop(Connection& conn, Client& client, json::Dictionary const& args);

      ///
      /// Refresh all networks (calls meta).
      ///   Q: {
      ///       "command": "refresh_networks",
      ///       "_id": "watchdog id",
      ///   }
      ///
      ///   R: no response
      ///
      void _on_refresh_networks(Connection& conn,
                                Client& client,
                                json::Dictionary const& args);

      ///
      /// Retreive file infos
      ///  Q : {
      ///       "command": "status",
      ///       "_id": "watchdog id",
      ///  }
      ///
      ///  R : {
      ///       "networks": [
      ///           {
      ///               "network_id": "the network id",
      ///               "mount_point": "/path/to/mount/point",
      ///           },
      ///       ],
      ///  }
      void _on_status(Connection& conn,
                      Client& client,
                      json::Dictionary const& args);

    };

  }
}



#endif /* ! CLIENTACTIONS_HH */


