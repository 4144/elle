#ifndef AWS_CREDENTIALS_HH
# define AWS_CREDENTIALS_HH

# include <string>

# include <boost/date_time/posix_time/posix_time.hpp>
# include <boost/optional.hpp>

# include <elle/attribute.hh>
# include <elle/Printable.hh>
# include <elle/serialization/fwd.hh>

# include <aws/Keys.hh>

namespace aws
{
  class Credentials:
    public elle::Printable
  {
      /*-------------.
      | Construction |
      `-------------*/
    public:
      Credentials() = default;
      /// Constructor for federated user.
      Credentials(std::string const& access_key_id,
                  std::string const& secret_access_key,
                  std::string const& session_token,
                  std::string const& region,
                  std::string const& bucket,
                  std::string const& folder,
                  boost::posix_time::ptime expiration,
                  boost::posix_time::ptime server_time);
      /// Constructor for normal user (i.e.: No session_token).
      Credentials(std::string const& access_key_id,
                  std::string const& secret_access_key,
                  std::string const& region,
                  std::string const& bucket,
                  std::string const& folder);
      std::string
      credential_string(RequestTime const& request_time,
                        Service const& aws_service);

      bool
      valid();

      ELLE_ATTRIBUTE_R(std::string, access_key_id);
      ELLE_ATTRIBUTE_R(std::string, secret_access_key);
      ELLE_ATTRIBUTE_R(boost::optional<std::string>, session_token);
      ELLE_ATTRIBUTE_R(std::string, region);
      ELLE_ATTRIBUTE_R(std::string, bucket);
      ELLE_ATTRIBUTE_R(std::string, folder);
      // Amazon current time from server
      ELLE_ATTRIBUTE_R(boost::posix_time::ptime, server_time);
      ELLE_ATTRIBUTE_R(boost::posix_time::ptime, expiry);
      // Estimated skew between trusted server time and local universal time.
      ELLE_ATTRIBUTE_RW(boost::posix_time::time_duration, skew);
      ELLE_ATTRIBUTE_R(bool, federated_user);

      /*--------------.
      | Serialization |
      `--------------*/
    public:
      Credentials(elle::serialization::SerializerIn& s);
      void
      serialize(elle::serialization::Serializer& s);

      /*----------.
      | Printable |
      `----------*/
    public:
      void
      print(std::ostream& stream) const;

    private:
      void
      _initialize(); // compute skew and expiry from input data
  };
}

#endif
