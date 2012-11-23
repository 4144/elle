#ifndef  PLASMA_WATCHDOG_MANAGER_HH
# define PLASMA_WATCHDOG_MANAGER_HH

# include <unordered_map>
# include <memory>
# include <functional>

# include <QCoreApplication>
# include <QTimer>
# include <QVariantMap>
# include <QVariantList>

# include "plasma/meta/Client.hh"

namespace plasma
{
  namespace watchdog
  {

    class Connection;
    class Client;
    class ClientActions;
    class NetworkManager;
    class LocalServer;

    ///
    /// The manager dispatch received command and stores clients with
    /// and their connection.
    ///
    class Manager
      : public QObject
    {
      Q_OBJECT

    public:
      typedef std::shared_ptr<Connection> ConnectionPtr;
      typedef std::unique_ptr<Client> ClientPtr;
      typedef std::unique_ptr<ClientActions> ClientActionsPtr;
      typedef std::unordered_map<ConnectionPtr, ClientPtr> ClientMap;

      typedef std::function<void(Connection&, Client&, QVariantMap const&)> Command;
      typedef std::unordered_map<std::string, Command> CommandMap;

      typedef plasma::meta::Client MetaClient;

    private:
      QCoreApplication&   _app;
      ClientMap*          _clients;
      CommandMap*         _commands;
      ClientActions*      _actions;
      NetworkManager*     _network_manager;
      MetaClient          _meta;
      LocalServer&        _local_server;
      QTimer              _timer;
      std::string         _identity;
      std::string         _user;
      std::string         _user_id;

    public:
      /// ctor & dtor
      Manager(QCoreApplication& app, LocalServer& local_server);
      ~Manager();

      /// properties
      MetaClient& meta()                                { return this->_meta; }

      NetworkManager& network_manager()     { return *this->_network_manager; }

      void token(QByteArray const& token);
      void token(QString const& token)        { this->token(token.toAscii()); }

      std::string const& identity() const           { return this->_identity; }
      void identity(std::string const& id)            { this->_identity = id; }
      void identity(QString const& id)  { this->_identity = id.toStdString(); }

      void user(std::string const& user)                { this->_user = user; }
      void user(QString const& user)      { this->_user = user.toStdString(); }
      std::string const& user() const                   { return this->_user; }

      void user_id(std::string const& user_id)                { this->_user_id = user_id; }
      void user_id(QString const& user_id)      { this->_user_id = user_id.toStdString(); }
      std::string const& user_id() const                   { return this->_user_id; }

      /// Called from the LocalServer to add a new connection
      Client& register_connection(ConnectionPtr& conn);
      void unregister_connection(ConnectionPtr& conn);

      ///
      /// Used by ClientActions to register callbacks. Any class may
      /// use it, but only one hook per command is allowed.
      ///
      void register_command(std::string const& id, Command cmd);
      void unregister_command(std::string const& id);
      void unregister_all_commands();

      /// Dispatch a command from a connection
      void execute_command(ConnectionPtr& conn, QVariantMap const& cmd);

      ///
      /// The manager is started from the class Application, but can
      /// be stopped anywhere.
      ///
      void start(std::string const& watchdogId);
      void stop();

      /// Start to refresh networks periodically.
      void start_refresh_networks();

      /// Force network refresh.
      void refresh_networks();

    private Q_SLOTS:
      void _on_timeout();
    };

  }
}



#endif /* ! MANAGER_HH */
