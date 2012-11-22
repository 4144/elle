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

#include <fcntl.h>

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
      {}
    };


    // FIXME: Stop desierializing item 'à la mano' from json dictionnary.

    ////////////////////////////////
    // User status: Login/Logout/AFK/...
    void
    Client::UserStatusHandler::_call(json::Dictionary const& dic,
                                     std::unique_ptr<Notification>&& notification,
                                     bool /* _new: This notification is 'instant' */)
    {
      ELLE_TRACE("Handling user status modification.");

      notification->user_id = dic["user_id"].as_string().value().c_str();
      notification->status = dic["status"].as_integer();

      this->callback(notification.get());
    }

    ////////////////////////////////
    // FileTransferHandler.
    void
    Client::TransactionHandler::_call(json::Dictionary const& dic,
                                      std::unique_ptr<Notification>&& notification,
                                      bool _new)
    {
      ELLE_TRACE("Handling new file transfer request.");

      notification->transaction_id = dic["transaction_id"].as_string().value().c_str();
      notification->first_filename = dic["first_filename"].as_string().value().c_str();
      notification->files_count = dic["files_count"].as_integer();
      notification->total_size = dic["total_size"].as_integer();
      notification->is_directory = dic["is_directory"].as_bool();
      notification->network_id = dic["network_id"].as_string().value().c_str();
      notification->sender_id = dic["sender_id"].as_string().value().c_str();
      notification->sender_fullname = dic["sender_fullname"].as_string().value().c_str();
      notification->recipient_id = dic["recipient_id"].as_string().value().c_str();
      notification->is_new = _new;

      ELLE_DEBUG("Deserialized successfuly.");
      this->callback(notification.get());
    }

    ////////////////////////////////
    // FileTransferStatusHandler.
    void
    Client::TransactionStatusHandler::_call(json::Dictionary const& dic,
                                            std::unique_ptr<Notification>&& notification,
                                            bool _new)
    {
      ELLE_TRACE("Handling file transfer status update.");

      notification->transaction_id = dic["transaction_id"].as_string().value().c_str();
      notification->network_id = dic["network_id"].as_string().value().c_str();
      notification->recipient_id = dic["recipient_id"].as_string().value().c_str();
      notification->sender_id = dic["sender_id"].as_string().value().c_str();
      notification->sender_device_id = dic["sender_device_id"].as_string().value().c_str();
      notification->recipient_device_id = dic["recipient_device_id"].as_string().value().c_str();
      notification->recipient_device_name = dic["recipient_device_name"].as_string().value().c_str();
      notification->status = dic["status"].as_integer();
      notification->is_new = _new;

      ELLE_DEBUG("Deserialized successfuly.");
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

      notification->sender_id = dic["sender_id"].as_string().value().c_str();
      notification->message = dic["message"].as_string().value().c_str();
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
      notification->debug = dic["debug"].as_string().value().c_str();
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
      ::fcntl(_impl->socket.native_handle(), F_SETFD, 1);
    }

    void
    Client::_read_socket()
    {
      boost::system::error_code err;
      auto old_available = _impl->response.in_avail();
      // Read socket.
      std::size_t size =  boost::asio::read_until(
        _impl->socket, _impl->response, "\n", err
      );
      auto new_available = _impl->response.in_avail();
      if (new_available != old_available)
        {
          if (size)
            ELLE_DEBUG("Read %s bytes from the socket (will use %s)",
                       new_available - old_available,
                       size);
          else
            ELLE_DEBUG("Read %s bytes from the socket (still not enough to build an object)",
                       new_available - old_available);
        }
      else
        ELLE_DEBUG("Nothing to read");

      if (err)
        {
          if (err != boost::asio::error::would_block)
            throw elle::HTTPException{elle::ResponseCode::error,
                elle::sprintf("Reading socket error: '%s'", err)
            };
          else
            return;
        }
      if (size == 0)
        return;
      try
        {
          ELLE_DEBUG("Recieved stream from trophonius.");

          // Bind stream to response.
          std::istream is(&(_impl->response));

          // Transfer socket stream to stringstream that ensure there are no
          // encoding troubles (and make the stream human readable).
          std::unique_ptr<char[]> data{new char[size]};
          if (!data)
            throw std::bad_alloc{};
          is.read(data.get(), size);
          std::stringstream ss{std::string{data.get(), size}};

          ELLE_DEGUG("Stream contains: '%s'.", std::string{data.get(), size});

          // while (!is.eof())
          // {
          //   std::string s;
          //   is >> s;

          //   ELLE_DEBUG("Stream is '%s'.", s);

          //   ::sleep(1);
          // }

          auto tmp = elle::format::json::parse(ss);

          _notifications.push(&tmp->as_dictionary());

          tmp.release();
        }
      catch (std::runtime_error const& err)
        {
          throw elle::HTTPException{
            elle::ResponseCode::bad_content, err.what()
          };
        }
    }

    bool
    Client::connect(std::string const& _id,
                    std::string const& token)
    {
      json::Dictionary connection_request{std::map<std::string, std::string>{
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
          ELLE_TRACE("Pop notification dictionnary to be handle.");

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
