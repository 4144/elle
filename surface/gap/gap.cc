#include <cassert>
#include <cstdlib>

#include <elle/log.hh>
#include <elle/elle.hh>
#include <elle/HttpClient.hh>

#include <lune/Lune.hh>

#include <elle/idiom/Close.hh>

#include "gap.h"
#include "State.hh"

#include <unordered_set>
#include <string.h>

ELLE_LOG_COMPONENT("infinit.surface.gap");

extern "C"
{

  /// - Utils -----------------------------------------------------------------

#define __TO_C(st)    reinterpret_cast<gap_State*>(st)
#define __TO_CPP(st)  reinterpret_cast<surface::gap::State*>(st)

# define CATCH_ALL(_func_)                                                     \
  catch (elle::HTTPException const& err)                                       \
    {                                                                          \
      ELLE_ERR(#_func_ " error: %s", err.what());                              \
      if (err.code == elle::ResponseCode::error)                               \
        ret = gap_network_error;                                               \
      else if (err.code == elle::ResponseCode::internal_server_error)          \
        ret = gap_api_error;                                                   \
      else                                                                     \
        ret = gap_internal_error;                                              \
    }                                                                          \
  catch (surface::gap::Exception const& err)                                   \
    {                                                                          \
      ELLE_ERR(#_func_ " error: %s", err.what());                              \
      ret = err.code;                                                          \
    }                                                                          \
  catch (std::exception const& err)                                            \
    {                                                                          \
      ELLE_ERR(#_func_ " error: %s", err.what());                              \
      ret = gap_internal_error;                                                \
    }                                                                          \
  /**/

// automate cpp wrapping
# define __WRAP_CPP_RET(_state_, _func_, ...)                                  \
  assert(_state_ != nullptr);                                                  \
  gap_Status ret;                                                              \
  try                                                                          \
    { __TO_CPP(_state_)->_func_(__VA_ARGS__); ret = gap_ok; }                  \
  CATCH_ALL(_func_)                                                            \
  /**/

# define __WRAP_CPP(...)                                                       \
  __WRAP_CPP_RET(__VA_ARGS__);                                                 \
  return ret                                                                   \
    /**/

  static char**
  _cpp_stringlist_to_c_stringlist(std::list<std::string> const& list)
  {
    size_t total_size = (1 + list.size()) * sizeof(void*);
    for (auto const& str : list)
      total_size += str.size() + 1;

    char** ptr = reinterpret_cast<char**>(malloc(total_size));
    if (ptr == nullptr)
      return nullptr;


    char** array = ptr;
    char* cstr = reinterpret_cast<char*>(ptr + (list.size() + 1));
    for (auto const& str : list)
      {
        *array = cstr;
        ::strncpy(cstr, str.c_str(), str.size() + 1);
        ++array;
        cstr += str.size() + 1;
      }
    *array = nullptr;
    return ptr;
  }

  /// - gap ctor & dtor -----------------------------------------------------

  gap_State* gap_new()
  {
    static bool initialized = false;
    if (!initialized)
      {
        initialized = true;
        if (lune::Lune::Initialize() == elle::Status::Error)
          {
            ELLE_ERR("Cannot initialize root components");
            return nullptr;
          }
      }

    try
      {
        return __TO_C(new surface::gap::State());
      }
    catch (std::exception const& err)
      {
        ELLE_ERR("Cannot initialize gap state: %s", err.what());
        return nullptr;
      }
  }

  gap_State* gap_new_with_token(char const* token)

{    static bool initialized = false;
    if (!initialized)
      {
        initialized = true;
        if (lune::Lune::Initialize() == elle::Status::Error)
          {
            ELLE_ERR("Cannot initialize root components");
            return nullptr;
          }
      }

    try
      {
        return __TO_C(new surface::gap::State(token));
      }
    catch (std::exception const& err)
      {
        ELLE_ERR("Cannot initialize gap state: %s", err.what());
        return nullptr;
      }
  }

  void gap_free(gap_State* state)
  {
    delete __TO_CPP(state);
  }

  void gap_enable_debug(gap_State* state)
  {
    assert(state != nullptr);
    // FIXME
    //__TO_CPP(state)->log.level(elle::log::Logger::Level::debug);
  }

  gap_Status gap_meta_status(gap_State*)
  {
    return gap_ok;
  }

  gap_Status
  gap_debug(gap_State* state)
  {
    (void) state;
    return gap_ok;
  }


  gap_Status
  gap_invite_user(gap_State* state,
                  char const* email)
  {
    __WRAP_CPP(state, invite_user, email);
  }

  gap_Status
  gap_message(gap_State* state,
              const char* recipient_id,
              const char* message)
  {
    assert(recipient_id != nullptr);
    assert(message != nullptr);
    __WRAP_CPP(state, send_message, recipient_id, message);
  }

  /// - Authentication ------------------------------------------------------

  char* gap_hash_password(gap_State* state,
                          char const* email,
                          char const* password)
  {
    assert(state != nullptr);
    assert(email != nullptr);
    assert(password != nullptr);

    try
      {
        std::string h = __TO_CPP(state)->hash_password(email, password);
        return ::strdup(h.c_str());
      }
    catch (std::exception const& err)
      {
        ELLE_ERR("Couldn't hash the password: %s", err.what());
      }
    return nullptr;
  }

  void gap_hash_free(char* h)
  {
    ::free(h);
  }

  gap_Status gap_login(gap_State* state,
                       char const* email,
                       char const* password)
  {
    assert(email != nullptr);
    assert(password != nullptr);
    __WRAP_CPP(state, login, email, password);
  }

  gap_Status gap_logout(gap_State* state)
  {
    __WRAP_CPP(state, logout);
  }

  gap_Status gap_register(gap_State* state,
                          char const* fullname,
                          char const* email,
                          char const* password,
                          char const* device_name,
                          char const* activation_code)
  {
    assert(fullname != nullptr);
    assert(email != nullptr);
    assert(password != nullptr);
    assert(activation_code != nullptr);

    __WRAP_CPP_RET(state, register_, fullname, email,
                   password, activation_code);

    if (ret == gap_ok && device_name != nullptr)
      {
        __WRAP_CPP(state, update_device, device_name, true);
      }
    return ret;
  }

  gap_Status
  gap_trophonius_connect(gap_State* state)
  {
    __WRAP_CPP_RET(state, connect);

    return ret;
  }


  gap_Status
  gap_poll(gap_State* state)
  {
    __WRAP_CPP_RET(state, poll);

    return ret;
  }
  /// - Device --------------------------------------------------------------
  gap_Status gap_device_status(gap_State* state)
  {
    try
      {
        if (__TO_CPP(state)->has_device())
          return gap_ok;
        else
          return gap_no_device_error;
      }
    catch (surface::gap::Exception const& err)
      {
        ELLE_ERR("Couldn't check the device: %s", err.what());
        return err.code;
      }
    catch (std::exception const& err)
      {
        ELLE_ERR("Couldn't check the device: %s", err.what());
      }
    return gap_internal_error;
  }

  gap_Status gap_set_device_name(gap_State* state,
                                 char const* name)
  {
    assert(name != nullptr);
    __WRAP_CPP(state, update_device, name);
  }

  /// - Network -------------------------------------------------------------

  char** gap_networks(gap_State* state)
  {
    assert(state != nullptr);
    gap_Status ret = gap_ok;
    try
      {
        auto const& networks_map = __TO_CPP(state)->networks();

        std::list<std::string> res;

        for (auto const& network_pair : networks_map)
          res.push_back(network_pair.first);

        return _cpp_stringlist_to_c_stringlist(res);
      }
    CATCH_ALL(networks);

    (void) ret;
    return nullptr;
  }

  void gap_networks_free(char** networks)
  {
    ::free(networks);
  }

  char const* gap_network_name(gap_State* state, char const* id)
  {
    assert(state != nullptr);
    assert(id != nullptr);
    gap_Status ret;
    try
      {
        auto const& network = __TO_CPP(state)->network(id);
        return network.name.c_str();
      }
    CATCH_ALL(network_name);

    (void) ret;
    return nullptr;
  }

  char const* gap_network_mount_point(gap_State* state,
                                      char const* id)
  {
    assert(state != nullptr);
    assert(id != nullptr);
    gap_Status ret;
    try
      {
        auto const& network_status = __TO_CPP(state)->network_status(id);
        return network_status.mount_point.c_str();
      }
    CATCH_ALL(network_mount_point);

    (void) ret;
    return nullptr;
  }

  char** gap_network_users(gap_State* state, char const* id)
  {
    assert(state != nullptr);
    assert(id != nullptr);
    gap_Status ret;
    try
      {
        auto const& network = __TO_CPP(state)->network(id);
        return _cpp_stringlist_to_c_stringlist(network.users);
      }
    CATCH_ALL(network_users);

    (void) ret;
    return nullptr;
  }

  void gap_network_users_free(char** users)
  {
    ::free(users);
  }

  gap_Status gap_create_network(gap_State* state,
                                char const* name)
  {
    __WRAP_CPP(state, create_network, name);
  }

  gap_Status
  gap_network_add_user(gap_State* state,
                       char const* network_id,
                       char const* user_id)
  {
    assert(network_id != nullptr);
    assert(user_id != nullptr);
    __WRAP_CPP(state, network_add_user, network_id, user_id);
  }


  /// - Self ----------------------------------------------------------------
  char const*
  gap_user_token(gap_State* state)
  {
    assert(state != nullptr);
    gap_Status ret;
    try
      {
        auto token = __TO_CPP(state)->get_token();
        return token.c_str();
      }
    CATCH_ALL(user_token);

    (void) ret;
    return nullptr;
  }

  char const*
  gap_self_id(gap_State* state)
  {
    assert(state != nullptr);
    gap_Status ret;
    try
      {
        auto user_id = __TO_CPP(state)->get_me()._id;
        return user_id.c_str();
      }
    CATCH_ALL(user_token);

    (void) ret;
    return nullptr;
  }

  /// - User ----------------------------------------------------------------

  char const* gap_user_fullname(gap_State* state, char const* id)
  {
    assert(state != nullptr);
    assert(id != nullptr);
    gap_Status ret;
    try
      {
        auto const& user = __TO_CPP(state)->user(id);
        return user.fullname.c_str();
      }
    CATCH_ALL(user_fullname);

    (void) ret;
    return nullptr;
  }

  char const* gap_user_email(gap_State* state, char const* id)
  {
    assert(state != nullptr);
    assert(id != nullptr);
    gap_Status ret;
    try
      {
        auto const& user = __TO_CPP(state)->user(id);
        return user.email.c_str();
      }
    CATCH_ALL(user_email);

    (void) ret;
    return nullptr;
  }

  char const* gap_user_by_email(gap_State* state, char const* email)
  {
    assert(state != nullptr);
    assert(email != nullptr);
    gap_Status ret;
    try
      {
        auto const& user = __TO_CPP(state)->user(email);
        return user._id.c_str();
      }
    CATCH_ALL(user_by_email);

    (void) ret;
    return nullptr;
  }

  char** gap_search_users(gap_State* state, char const* text)
  {
    assert(state != nullptr);
    text = ""; //XXX text search not used
    assert(text != nullptr);
    gap_Status ret;
    try
      {
        auto users = __TO_CPP(state)->search_users(text);
        std::list<std::string> result;
        for (auto const& pair : users)
          result.push_back(pair.first);
        return _cpp_stringlist_to_c_stringlist(result);
      }
    CATCH_ALL(search_users);

    (void) ret;
    return nullptr;
  }

  void gap_search_users_free(char** users)
  {
    ::free(users);
  }

  /// - Watchdog ------------------------------------------------------------

  gap_Status gap_launch_watchdog(gap_State* state)
  {
    __WRAP_CPP(state, launch_watchdog);
  }

  gap_Status gap_refresh_networks(gap_State* state)
  {
    __WRAP_CPP(state, refresh_networks);
  }

  gap_Status gap_stop_watchdog(gap_State* state)
  {
    __WRAP_CPP(state, stop_watchdog);
  }

  /// - Permissions ---------------------------------------------------------

  gap_Status gap_set_permissions(gap_State* state,
                                 char const* user_id,
                                 char const* absolute_path,
                                 int permissions)
  {
    assert(user_id != nullptr);
    assert(absolute_path != nullptr);
    __WRAP_CPP(state, set_permissions, user_id, absolute_path, permissions);
  }

  gap_Status gap_set_permissions_rec(gap_State* state,
                                     char const* user_id,
                                     char const* absolute_path,
                                     int permissions)
  {
    assert(user_id != nullptr);
    assert(absolute_path != nullptr);
    __WRAP_CPP(state,
               set_permissions,
               user_id,
               absolute_path,
               permissions,
               true);
  }

  char** gap_file_users(gap_State* state,
                        char const* absolute_path)
  {
    assert(absolute_path != nullptr);
    gap_Status ret;
    try
      {
        auto const& infos = __TO_CPP(state)->file_infos(absolute_path);
        std::list<std::string> list;
        for (auto const& pair : infos.accesses)
          list.push_back(pair.first);
        return _cpp_stringlist_to_c_stringlist(list);
      }
    CATCH_ALL(get_file_users);

    (void) ret;
    return nullptr;
  }

  void gap_file_users_free(char** users)
  {
    ::free(users);
  }

  int gap_get_permissions(gap_State* state,
                          char const* user_id,
                          char const* absolute_path)
  {
    assert(user_id != nullptr);
    assert(absolute_path != nullptr);
    gap_Status ret;
    try
      {
        auto const& infos = __TO_CPP(state)->file_infos(absolute_path);
        auto it = infos.accesses.find(user_id);
        if (it == infos.accesses.end())
          throw surface::gap::Exception(gap_error, "No such user for this file");
        return it->second;
      }
    CATCH_ALL(get_permissions);

    (void) ret;
    return gap_error;
  }

  // - Trophonius -----------------------------------------------------------

  gap_Status
  gap_user_status_callback(gap_State* state,
                           gap_user_status_callback_t cb)
  {
    gap_Status ret = gap_ok;
    try
      {
        __TO_CPP(state)->attach_callback(
          std::function<void (gap_UserStatusNotification const*)>(cb)
        );
      }
    CATCH_ALL(user_status_callback);

    return ret;
  }

  gap_Status
  gap_transaction_callback(gap_State* state,
                                   gap_transaction_callback_t cb)
  {
    gap_Status ret = gap_ok;
    try
      {
        __TO_CPP(state)->attach_callback(
          std::function<void (gap_TransactionNotification const*)>(cb)
        );
      }
    CATCH_ALL(transaction_callback);

    return ret;
  }

  gap_Status
  gap_transaction_status_callback(gap_State* state,
                                  gap_transaction_status_callback_t cb)
  {
    gap_Status ret = gap_ok;
    try
      {
        __TO_CPP(state)->attach_callback(
          std::function<void (gap_TransactionStatusNotification const*)>(cb)
        );
      }
    CATCH_ALL(transaction_status_callback);

    return ret;
  }

  gap_Status
  gap_message_callback(gap_State* state,
                       gap_message_callback_t cb)
  {
    gap_Status ret = gap_ok;
    try
      {
        __TO_CPP(state)->attach_callback(
          std::function<void (gap_MessageNotification const*)>(cb)
        );
      }
    CATCH_ALL(message_callback);

    return ret;
  }

  // - Notifications -----------------------------------------------------------

  gap_Status
  gap_pull_notifications(gap_State* state,
                         int count,
                         int offset)
  {
    __WRAP_CPP(state, pull_notifications, count, offset);
  }

  gap_Status
  gap_notifications_read(gap_State* state)
  {
    __WRAP_CPP(state, notifications_read);
  }

 char** gap_transactions(gap_State* state)
  {
    assert(state != nullptr);
    gap_Status ret = gap_ok;
    try
      {
        auto const& transactions_map = __TO_CPP(state)->transactions();

        std::list<std::string> res;

        for (auto const& transaction_pair : transactions_map)
          res.push_back(transaction_pair.first);

        return _cpp_stringlist_to_c_stringlist(res);
      }
    CATCH_ALL(transactions);

    (void) ret;
    return nullptr;
  }

  void gap_transactions_free(char** transactions)
  {
    ::free(transactions);
  }

  gap_Status
  gap_send_files(gap_State* state,
                 char const* recipient_id,
                 char const* const* files)
  {
    assert(state != nullptr);
    assert(recipient_id != nullptr);
    assert(files != nullptr);

    gap_Status ret = gap_ok;

    try
      {
        std::unordered_set<std::string> s;

        while(*files != nullptr)
          {
            s.insert(*files);
            ++files;
          }

        __TO_CPP(state)->send_files(recipient_id,
                                    s);
      }
    CATCH_ALL(send_files)

      return ret;
  }

  gap_Status
  gap_update_transaction(gap_State* state,
                         char const* transaction_id,
                         gap_TransactionStatus status)
  {
    assert(transaction_id != nullptr);
    assert(   status > gap_TransactionStatus::gap_transaction_status_none
           && status < gap_TransactionStatus::_gap_transaction_status_count);

    __WRAP_CPP_RET(state,
                   update_transaction,
                   transaction_id,
                   status);

    return ret;
  }

  gap_Status
  gap_set_output_dir(gap_State* state,
                     char const* output_path)
  {
    assert(state != nullptr);
    assert(output_path != nullptr);

    __WRAP_CPP_RET(state,
                   output_dir,
                   output_path);

    return ret;
  }

  gap_TransactionStatus
  gap_transaction_status(gap_State* state,
                         char const* transaction_id)
  {
    assert(state != nullptr);
    assert(transaction_id != nullptr);
    gap_Status ret;

    try
      {
        auto const& transaction = __TO_CPP(state)->transaction(transaction_id);
        return (gap_TransactionStatus) transaction.status;
      }
    CATCH_ALL(transaction_status);

    (void) ret;
    return gap_TransactionStatus::gap_transaction_status_none;
  }

  char const*
  gap_transaction_owner(gap_State* state,
                        char const* transaction_id)
  {
    assert(state != nullptr);
    assert(transaction_id != nullptr);

    gap_Status ret;
    try
      {
        auto const& transaction = __TO_CPP(state)->transaction(transaction_id);
        return transaction.sender_id.c_str();
      }
    CATCH_ALL(transaction_owned);

    (void) ret;
    return nullptr;
  }

} // ! extern "C"
