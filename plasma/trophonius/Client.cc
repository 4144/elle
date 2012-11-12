#include <elle/log.hh>
#include <elle/serialize/JSONArchive.hh>
#include <elle/format/json/Dictionary.hxx>
#include <elle/format/json/Parser.hh>
#include <elle/serialize/ListSerializer.hxx>

#include "Client.hh"

#include <iostream>
#include <fstream>


#include <elle/idiom/Close.hh>

#include <elle/print.hh>

ELLE_LOG_COMPONENT("infinit.plasma.trophonius.Client");

namespace plasma
{
  namespace trophonius
  {
    struct Client::Impl
    {
      boost::asio::io_service io_service;

      /// The connexion to the server
      boost::asio::ip::tcp::socket socket;
      std::string             server;
      uint16_t                port;
      bool                    check_errors;
      boost::asio::streambuf request;
      boost::asio::streambuf response;

      Impl(std::string const& server,
           uint16_t port,
           bool check_errors)
        : io_service{}
        , socket{io_service}
        , server{server}
        , port{port}
        , check_errors{check_errors}
        , request{}  // Use once to initiate connection.
        , response{}
      {
      }
    };


    // FIXME: Stop desierializing item 'à la mano' from json dictionnary.

    ////////////////////////////////
    // User status: Login/Logout/AFK/...
    void
    Client::UserStatusHandler::_call(json::Dictionary const& dic,
                                     std::unique_ptr<Notification>&& notification,
                                     bool _new)
    {
      ELLE_TRACE("Handling user status modification.");

      // This notification is 'instant'.
      (void) _new;

      std::string temp = dic["sender_id"].as_string();
      notification->sender_id = temp.c_str();

      notification->status = dic["status"].as_integer();

      this->callback(notification.get());
    }

    ////////////////////////////////
    // FileTransferHandler.
    void
    Client::FileTransferRequestHandler::_call(json::Dictionary const& dic,
                                              std::unique_ptr<Notification>&& notification,
                                              bool _new)
    {
      ELLE_TRACE("Handling new file transfer request.");

      std::string temp = dic["transaction_id"].as_string();
      notification->transaction_id = temp.c_str();

      temp = dic["sender_id"].as_string();
      notification->sender_id = temp.c_str();

      temp = dic["file_name"].as_string();
      notification->file_name = temp.c_str();

      notification->file_size = dic["file_size"].as_integer();

      notification->is_new = _new;

      this->callback(notification.get());
    }

    ////////////////////////////////
    // FileTransferStatusHandler.
    void
    Client::FileTransferStatusHandler::_call(json::Dictionary const& dic,
                                             std::unique_ptr<Notification>&& notification,
                                             bool _new)
    {
      ELLE_TRACE("Handling file transfer status update.");

      std::string temp = dic["network_id"].as_string();
      notification->network_id = temp.c_str();

      notification->status = dic["status"].as_integer();

      notification->is_new = _new;

      this->callback(notification.get());
    }

    ////////////////////////////////
    // Message.
    void
    Client::MessageHandler::_call(json::Dictionary const& dic,
                                  std::unique_ptr<Notification>&& notification,
                                  bool _new)
    {
      ELLE_TRACE("Handling new message.");

      std::string temp = dic["sender_id"].as_string();
      notification->sender_id = temp.c_str();

      temp = dic["message"].as_string();
      notification->message = temp.c_str();

      notification->is_new = _new;

      // Use callback function.
      this->callback(notification.get());
    }


    ////////////////////////////////
    // BiteHandler.
    void
    Client::BiteHandler::_call(json::Dictionary const& dic,
                               std::unique_ptr<Notification>&& notification,
                               bool _new)
    {
      std::string temp = dic["debug"].as_string();
      notification->debug = temp.c_str();

      notification->is_new = _new;

      // Use callback function.
      this->callback(notification.get());
    }

    Client::Client(std::string const& server,
                   uint16_t port,
                   bool check_errors)
      : _impl{new Impl{server, port, check_errors}}
    {
      // Resolve the host name into an IP address.
      boost::asio::ip::tcp::resolver resolver(_impl->io_service);
      boost::asio::ip::tcp::resolver::query query(_impl->server,
        elle::sprint(_impl->port));
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
        resolver.resolve(query);

      // Start connect operation.
      _impl->socket.connect(*endpoint_iterator);

      _impl->socket.non_blocking(true);
    }

    void
    Client::_read_socket()
    {
      boost::system::error_code err;
      // Read socket.
      std::size_t size =  boost::asio::read_until(_impl->socket,
                                                  _impl->response,
                                                  '\n',
                                                  err);

      if(!err && size != 0)
        {
          try
            {
              // Bind stream to response.
              std::istream is(&(_impl->response));
              // Don't skip whitespaces in the stream.
              is >> std::noskipws;

              std::stringstream debug_ss;

              // Copy stream to streambuff.
              std::copy(
                std::istream_iterator<char>(is),
                std::istream_iterator<char>(),
                std::ostream_iterator<char>(debug_ss)
                );

              auto tmp = elle::format::json::parse(debug_ss);

              _notifications.push(dynamic_cast<json::Dictionary*>(tmp.get()));

              tmp.release();
            }
          catch (std::exception const& err)
            {
              throw elle::HTTPException(elle::ResponseCode::bad_content, err.what());
            }
        }
      else if (err != boost::asio::error::would_block)
        {
          // An important error occurred.
          throw elle::HTTPException(elle::ResponseCode::error,
                                    elle::sprintf("Reading socket error: '%s'",
                                                  err));
        }
    }

    bool
    Client::connect(std::string const& _id,
                    std::string const& token)
    {
      json::Dictionary connection_request{std::map<std::string, std::string>
      {
        {"_id", _id},
        {"token", token},
      }};

      std::ostream request_stream(&_impl->request);

      // May raise an exception.
      elle::serialize::OutputJSONArchive(request_stream, connection_request);

      // Add '\n' to request.
      request_stream << std::endl;

      boost::system::error_code err;

      boost::asio::write(
        _impl->socket,
        _impl->request,
        err
        );

      if (!err)
        return true;

      //An error occurred.
      throw elle::HTTPException(elle::ResponseCode::error, "Writting socket error");

      return false;
    }

    std::unique_ptr<json::Dictionary>
    Client::poll()
    {
      // Check socket and insert notification dictionary in the queue if any.
      _read_socket();

      std::unique_ptr<json::Dictionary> ret;

      if (!_notifications.empty())
        {
          // Fill dictionary.
          ret.reset(_notifications.front());
          _notifications.pop();
        }

      return ret;
    }

    bool
    Client::has_notification(void)
    {
      return !(_notifications.empty());
    }

  }
}
