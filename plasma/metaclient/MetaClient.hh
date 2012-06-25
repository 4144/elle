#ifndef PLASMA_METCLIENT_METACLIENT_HH
# define PLASMA_METCLIENT_METACLIENT_HH

# include <functional>
# include <list>
# include <map>
# include <stdexcept> // XXX

# include <QCoreApplication>
# include <QApplication>
# include <QNetworkAccessManager>
# include <QNetworkReply>
# include <QVariantMap>

# include <elle/print.hh>

namespace plasma
{
  namespace metaclient
  {

    /// Base class for every response
    struct Response
    {
      bool success;
      std::string error;
    };

    /// Login response
    struct LoginResponse : Response
    {
      std::string  token;
      std::string  fullname;
      std::string  email;
      std::string  identity;
    };

    struct NetworksResponse : Response
    {
      std::list<std::string> networks;
    };

    struct NetworkResponse : Response
    {
      std::string              _id;
      std::string              name;
      std::string              model;
      std::string              root_block;
      std::string              root_address;
      std::string              descriptor;
      std::list<std::string>   devices;
      std::list<std::string>   users;
    };

    struct CreateDeviceResponse : Response
    {
      std::string             created_device_id;
      std::string             passport;
    };

    struct UpdateDeviceResponse : Response
    {
      std::string             updated_device_id;
      std::string             passport;
    };

    struct UpdateNetworkResponse : Response
    {
      std::string             updated_network_id;
      std::string             descriptor;
      std::string             root_block;
      std::string             root_address;
    };

    struct NetworkNodesResponse : Response
    {
      std::string             network_id;
      std::list<std::string>  nodes;
    };

    ///
    /// Convenient interface to the meta server
    ///
    class MetaClient : public QObject
    {
      Q_OBJECT

    public:
      enum class Error
      {
        ConnectionFailure,
        InvalidContent,
        ServerError,
        CallbackError,
      };

      /// Callbacks for API calls
      typedef std::function<void(LoginResponse const&)> LoginCallback;
      typedef std::function<void(NetworksResponse const&)> NetworksCallback;
      typedef std::function<void(NetworkResponse const&)> NetworkCallback;
      typedef std::function<void(CreateDeviceResponse const&)> CreateDeviceCallback;
      typedef std::function<void(UpdateDeviceResponse const&)> UpdateDeviceCallback;
      typedef std::function<void(UpdateNetworkResponse const&)> UpdateNetworkCallback;
      typedef std::function<void(NetworkNodesResponse const&)> NetworkNodesCallback;

      typedef std::function<void(Error, std::string const&)> Errback;
    public:
      struct RequestHandler;
    protected:
      typedef std::map<QNetworkReply*, RequestHandler*> HandlerMap;

    private:
      /// Network loop
      QNetworkAccessManager _network;

      /// Connection token
      QByteArray            _token;

      /// Current requests handlers
      HandlerMap            _handlers;

    public:

      /// ctor & dtor
      MetaClient(QCoreApplication& app);
      MetaClient(QApplication& app);
      MetaClient();
      ~MetaClient();

      /// Each method represent an API call
      void Login(std::string const& email,
                 std::string const& password,
                 LoginCallback callback,
                 Errback errback = nullptr);

      LoginResponse Login(std::string const& email,
                          std::string const& password)
      {
        struct {
            LoginResponse response;
            void operator ()(LoginResponse const& response)
            { this->response = response; }
        } callback;

        struct {
            bool called;
            Error error;
            std::string string;
            void operator ()(Error error, std::string const string)
            {
              this->called = true;
              this->error = error;
              this->string = string;
            }
        } errback;

        QEventLoop loop;
        this->Login(email, password, callback, errback);
        QObject::connect(this, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        if (errback.called)
          throw std::runtime_error(
            elle::sprint("Got error", (int) errback.error,":", errback.string)
          );

        return callback.response;
      }

      void CreateDevice(std::string const& name,
                        std::string const& endpoint,
                        short port,
                        CreateDeviceCallback callback,
                        Errback errback = nullptr);

      void UpdateDevice(std::string const& _id,
                        char const* name,
                        char const* endpoint,
                        short port,
                        UpdateDeviceCallback callback,
                        Errback errback = nullptr);

      void GetNetworks(NetworksCallback callback,
                       Errback errback = nullptr);

      void GetNetwork(std::string const& id,
                      NetworkCallback callback,
                      Errback errback = nullptr);

      void UpdateNetwork(std::string const& id,
                         std::string const* name,
                         std::list<std::string> const* users,
                         std::list<std::string> const* devices,
                         std::string const* rootBlock,
                         std::string const* rootAddress,
                         UpdateNetworkCallback callback,
                         Errback errback = nullptr);

      void GetNetworkNodes(std::string const& id,
                           NetworkNodesCallback cb,
                           Errback errback = nullptr);

      /// properties
      QByteArray const& token() const { return this->_token; }
      void token(QByteArray const& token) { this->_token = token; }

    private:
      void _Post(std::string const& url,
                 QVariantMap const& data,
                 RequestHandler* handler);
      void _Get(std::string const& url,
                RequestHandler* handler);

    private Q_SLOTS:
      void _OnRequestFinished(QNetworkReply* reply);

    Q_SIGNALS:
      void finished();
    };

  } // !metaclient
} // !plasma

#endif /* ! METACLIENT_HH */


