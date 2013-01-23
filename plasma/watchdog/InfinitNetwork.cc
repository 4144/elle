#include "InfinitNetwork.hh"
#include "Manager.hh"

#include <Infinit.hh>

#include <common/common.hh>

#include <elle/log.hh>
#include <elle/os/path.hh>
#include <elle/io/Piece.hh>
#include <elle/serialize/extract.hh>

#include <lune/Descriptor.hh>
#include <lune/Identity.hh>
#include <elle/Passport.hh>
#include <lune/Set.hh>
#include <lune/Lune.hh>

#include <nucleus/proton/Address.hh>
#include <nucleus/proton/Porcupine.hh>
#include <nucleus/neutron/Genre.hh>
#include <nucleus/neutron/Object.hh>
#include <nucleus/neutron/Trait.hh>
#include <nucleus/neutron/Subject.hh>
#include <nucleus/neutron/Access.hh>

#include <elle/idiom/Close.hh>

#include <hole/storage/Directory.hh>

#include <QDir>

#include <stdexcept>
#include <iostream>

#include <stdlib.h>
#include <sys/wait.h>


#define LOG(Fmt, ...)                                                         \
  ELLE_DEBUG("InfinitNetwork::%s(id=%s): " Fmt,                               \
             __FUNCTION__, this->_description._id, ##__VA_ARGS__);            \
  /**/

ELLE_LOG_COMPONENT("infinit.plasma.watchdog");

using namespace plasma::watchdog;
namespace path = elle::os::path;

InfinitNetwork::InfinitNetwork(Manager& manager,
                               meta::NetworkResponse const& response)
  : QObject()
  , _description(response)
  , _manager(manager)
  , _process()
  , _network_dir{
      common::infinit::network_directory(_manager.user_id(), response._id)
    }
  , _mount_point{path::join(_network_dir, "mnt")}
{
  LOG("Creating new network.");

  this->connect(
      &this->_process, SIGNAL(started()),
      this, SLOT(_on_process_started())
  );

  this->connect(
      &this->_process, SIGNAL(error(QProcess::ProcessError)),
      this, SLOT(_on_process_error(QProcess::ProcessError))
  );

  this->connect(
      &this->_process, SIGNAL(finished(int, QProcess::ExitStatus)),
      this, SLOT(_on_process_finished(int, QProcess::ExitStatus))
  );
  this->_update();
}

InfinitNetwork::~InfinitNetwork()
{
  if (this->_process.state() != QProcess::NotRunning)
    {
      ELLE_WARN("Network %s not terminated (sending SIGTERM)",
                this->_description.name);
      ::kill(this->_process.pid(), SIGKILL);
      this->_process.waitForFinished(100);
    }
}

std::string InfinitNetwork::mount_point() const
{
  return this->_mount_point;
}

std::string const& InfinitNetwork::id() const
{
  return this->_description._id;
}

void InfinitNetwork::update(meta::NetworkResponse const& response)
{
  LOG("Updating network: %s", response._id);
  this->_description = response;
  this->_update();
}

void InfinitNetwork::stop()
{
  ELLE_DEBUG("Shutting down network %s", this->_description.name);
  if (this->_process.state() == QProcess::Running)
    {
      ::kill(this->_process.pid(), SIGINT);
      this->_process.waitForFinished(3000);
    }
}

void InfinitNetwork::_update()
{
  LOG("Starting network update.");
  if (!path::exists(this->_network_dir))
    path::make_path(this->_network_dir);

  std::string description_filename = path::join(
      this->_network_dir,
      this->_description._id + ".dsc"
  );

  if (!path::exists(description_filename))
    {
      if (!this->_description.descriptor.size())
        return this->_create_network_root_block(this->_description._id);
      else
          this->_prepare_directory();
    }
  else
    this->_register_device();

  LOG("End of _update");
}

/// Called when the network does not have any descriptor
void InfinitNetwork::_create_network_root_block(std::string const& id)
{
  LOG("Creating the network descriptor.");
  // XXX this value depends on the network policy and openness.
  static nucleus::neutron::Permissions permissions =
    nucleus::neutron::permissions::read;

  auto e              = elle::Status::Error;
  auto genreDirectory = nucleus::neutron::Genre::directory;

  LOG("Create proton network from id '%s'.", id);
  nucleus::proton::Network network(id);

  //- identity ----------------------------------------------------------------
  lune::Identity                identity;
  if (identity.Restore(this->_manager.identity())             == e)
    throw std::runtime_error("Couldn't restore the identity.");

  //- group -------------------------------------------------------------------
  nucleus::neutron::Group group(network, identity.pair().K(), "everybody");
  group.seal(identity.pair().k());

  //- group address -----------------------------------------------------------
  nucleus::proton::Address      group_address(group.bind());

  //- subject -----------------------------------------------------------------
  nucleus::neutron::Subject subject;
  if (subject.Create(group_address) == elle::Status::Error)
    throw std::runtime_error("unable to create the group subject");

  //- access-------------------------------------------------------------------
  nucleus::proton::Porcupine<nucleus::neutron::Access> access_porcupine{
    nucleus::proton::nest::none()};

  nucleus::proton::Door<nucleus::neutron::Access> access_door =
    access_porcupine.lookup(subject);

  access_door.open();

  access_door().insert(new nucleus::neutron::Record{subject, permissions});

  access_door.close();

// XXX[cf: etoile/automaton/Access.hh>, until no longer encrypted]
#define ACCESS_SECRET_KEY_LENGTH 256
#define ACCESS_SECRET_KEY "no-secret-key"

  // XXX
  static cryptography::SecretKey secret_key{ACCESS_SECRET_KEY};

  ELLE_ASSERT(access_porcupine.strategy() == nucleus::proton::Strategy::value);

  cryptography::Digest access_fingerprint =
    nucleus::neutron::access::fingerprint(access_porcupine);

  nucleus::proton::Radix access_radix = access_porcupine.seal(secret_key);

  //- directory ---------------------------------------------------------------
  nucleus::neutron::Object      directory(network,
                                          identity.pair().K(),
                                          genreDirectory);

  if (directory.Update(directory.author(),
                       directory.contents(),
                       directory.size(),
                       access_radix,
                       directory.owner_token()) == e)
    throw std::runtime_error("unable to update the directory");

  if (directory.Seal(identity.pair().k(), access_fingerprint) == e)
    throw std::runtime_error("Cannot seal the access");

  //- directory address -------------------------------------------------------
  nucleus::proton::Address      directory_address(directory.bind());

  {
    // XXX[to improve: contact Raphael]
    elle::io::Unique root_block_;
    directory.Save(root_block_);
    elle::io::Unique root_address_;
    directory_address.Save(root_address_);

    elle::io::Unique access_block_;
    access.Save(access_block_);
    elle::io::Unique access_address_;
    access_address.Save(access_address_);

    elle::io::Unique group_block_;
    group.Save(group_block_);
    elle::io::Unique group_address_;
    group_address.Save(group_address_);

    this->_on_got_descriptor(this->_manager.meta().update_network(
                               this->_description._id,
                               nullptr,
                               &root_block_,
                               &root_address_,
                               &access_block_,
                               &access_address_,
                               &group_block_,
                               &group_address_
                               ));
  }
}

/// Prepare the network directory, store root block and network descriptor
void InfinitNetwork::_prepare_directory()
{
  LOG("Prepare network directory.");
  using elle::serialize::from_string;
  using elle::serialize::InputBase64Archive;

  elle::io::Path shelter_path(lune::Lune::Shelter);
  shelter_path.Complete(elle::io::Piece{"%USER%", this->_manager.user_id()},
                        elle::io::Piece{"%NETWORK%", this->_description._id});
  ELLE_DEBUG("Shelter path == %s", shelter_path.string());
  hole::storage::Directory storage(shelter_path.string());

  {
    LOG("Built directory storage of %s", this->_description._id);

    assert(this->_description.root_block.size());
    assert(this->_description.descriptor.size());

    LOG("Create lune descriptor of %s", this->_description._id);

    lune::Descriptor descriptor{
      from_string<InputBase64Archive>(_description.descriptor)
    };
    LOG("Lune descriptor created");

  // XXX[pas forcement necessaire si le format n'a pas change entre
  //     la version du descriptor et celle d'Infinit. il faudrait
  //     comparer static format avec ceux de reference dans le descriptor]
  //if (descriptor.version() > Infinit::version)
  //  {
  //    throw std::runtime_error("you have to update Infinit");
  //  }

  // XXX[ici plutot compare static/dynamic format car on peut passer
  //     a une nouvelle version sans que le descriptor n'ait change
  //     de format]
  //if (description.version() < Infinit::version && je suis owner)
  //  {
  //     static_assert(false, "migrate the descriptor here and send to meta");
  //  }

    lune::Identity identity;
    identity.Restore(this->_manager.identity());

    LOG("Storing the descriptor of %s for user %s", _description._id, _manager.user_id());
    descriptor.store(identity);

    nucleus::neutron::Object directory{
      from_string<InputBase64Archive>(_description.root_block)
    };

    storage.store(descriptor.meta().root(), directory);
    LOG("Root block stored.");
  }

  {
    LOG("Storing access block.");
    LOG("block: '%s'.", _description.access_block);
    nucleus::neutron::Access access{
      from_string<InputBase64Archive>(_description.access_block)
    };
    LOG("address: '%s'.", _description.access_address);
    nucleus::proton::Address access_address{
      from_string<InputBase64Archive>(_description.access_address)
    };
    LOG("Deserialization complete.");
    storage.store(access_address, access);
    LOG("Address block stored.");
  }

  {
    LOG("Storing group block.");
    LOG("block: '%s'.", _description.group_block);
    nucleus::neutron::Group group{
      from_string<InputBase64Archive>(_description.group_block)
    };
    LOG("address: '%s'.", _description.group_address);
    nucleus::proton::Address group_address{
      from_string<InputBase64Archive>(_description.group_address)
    };
    LOG("Deserialization complete.");
    storage.store(group_address, group);
    LOG("Group block stored.");
  }

  this->_register_device();

  LOG("End of prepare directory");
}


/// Append the local device to the network
void InfinitNetwork::_register_device()
{
  LOG("Check if the device is registered for this network.");
  elle::io::Path passport_path(lune::Lune::Passport);
  LOG("Complete passport path with user '%s'", this->_manager.user_id());
  passport_path.Complete(elle::io::Piece{"%USER%", this->_manager.user_id()});

  elle::Passport passport;
  passport.load(passport_path);

  this->_manager.meta().network_add_device(
    this->_description._id,
    passport.id()
  );

  this->_on_network_nodes(
    this->_manager.meta().network_nodes(this->_description._id)
  );

  LOG("End of _register_device.");
}

/// Update the network nodes set when everything is good
void InfinitNetwork::_on_network_nodes(meta::NetworkNodesResponse const& response)
{
  LOG("begin");
  lune::Set locusSet;

  auto it =  response.nodes.begin(),
       end = response.nodes.end();
  for (; it != end; ++it)
  {
    LOG("\t * %s", *it);
    elle::network::Locus locus;

    if (locus.Create(*it) == elle::Status::Error)
      throw std::runtime_error("Cannot create locus from string '" + *it + "'.");
    if (locusSet.Add(locus) == elle::Status::Error)
      {
        LOG("Cannot add locus '%s' to the set (ignored).", *it);
      }
  }

  locusSet.store(
    this->_manager.user_id(),
    this->_description._id
  );

  this->_start_process();

  LOG("end");
}
void InfinitNetwork::_on_got_descriptor(meta::UpdateNetworkResponse const& response)
{
  LOG("Got network descriptor.");
  if (response.updated_network_id != this->_description._id)
    {
      throw std::runtime_error(
          "mismatch ids... between updated '" +
          response.updated_network_id + "' and the old one '" +
          this->_description._id + "'"
      );
    }
  this->_description = this->_manager.meta().network(this->_description._id);
  LOG("Got descriptor for %s (%s): %s", this->_description.name,
      this->_description._id, this->_description.descriptor);

  this->_prepare_directory();
}

void InfinitNetwork::_on_any_error(elle::ResponseCode error, std::string const& err)
{
  LOG("Got error while creating the network '%s': %s (%s)",
      this->_description.name, err, (int)error);
}

void InfinitNetwork::_start_process()
{
  if (this->_process.state() != QProcess::NotRunning)
    return;
  LOG("Starting infinit process (mount point: %s)", this->_mount_point);

  std::string pid_file = path::join(this->_network_dir, "run.pid").c_str();

  LOG("Set out files paths.")
  if (elle::os::path::exists(pid_file))
    {
      pid_t pid = 0;

        {
          std::ifstream in(pid_file);
          if (in.good())
            in >> pid;
          in.close();
        }

      if (pid != 0)
        {
          if (kill(pid, SIGINT) == 0)
            {
              (void)kill(pid, SIGKILL);
            }
          sleep(1);
          char const* const umount_arguments[] = {
#ifdef INFINIT_MACOSX
              "umount",
              this->_mount_point.c_str(),
#endif
#ifdef INFINIT_LINUX
              "fusermount"
              "-z", "-u",
              this->_mount_point.c_str(),
#endif
              nullptr
          };
          pid_t umount_pid = fork();
          if (umount_pid == 0)
            {
              close(STDERR_FILENO);
              if (execvp(umount_arguments[0], (char**) umount_arguments) != 0)
                {
                  ELLE_ERR("Cannot launch %s", umount_arguments[0]);
                }
              exit(1);
            }
          else if (umount_pid > 0)
            {
              int status;
              (void) waitpid(umount_pid, &status, 0);
            }
        }

        {
          std::ofstream out(pid_file);
          if (out.good())
            out << 0;
          out.close();
        }
    }

  try
  {
    if (!path::exists(this->_mount_point))
      path::make_path(this->_mount_point);
  }
  catch(std::runtime_error const& e)
  {
    ELLE_WARN("mount point couldn't be created at '%s': %s",
              this->_mount_point, e.what());

    return;
  }

  ELLE_DEBUG("Create mount point link");
  {
    std::string mnt_link_dir = path::join(
      common::system::home_directory(),
      "Infinit"
    );

    if (!path::exists(mnt_link_dir))
      path::make_directory(mnt_link_dir);

    std::string owner_email = this->_manager.meta().user(
        this->_description.owner
    ).email;

    std::string mnt_link = path::join(
      mnt_link_dir,
      elle::sprintf("%s (%s)", this->_description.name, owner_email)
    );

    if (!path::exists(mnt_link))
      path::make_symlink(this->_mount_point, mnt_link);
  }

  LOG("exec: %s -n %s -m %s -u %s",
      common::infinit::binary_path("8infinit"),
      this->_description._id.c_str(),
      this->_mount_point,
      this->_manager.user_id().c_str());

  QStringList arguments;
  arguments << "-n" << this->_description._id.c_str()
            << "-m" << this->_mount_point.c_str()
            << "-u" << this->_manager.user_id().c_str()
            ;

  LOG("8infinit arguments created.")
  // XXX[rename into [network-name].log]
  std::string log_out = path::join(this->_network_dir, "out.log").c_str();
  std::string log_err = path::join(this->_network_dir, "err.log").c_str();

  LOG("Setting output files to process.");
  this->_process.setStandardOutputFile(log_out.c_str());
  this->_process.setStandardErrorFile(log_err.c_str());

  LOG("Run 8infinit.");
  this->_process.start(
      common::infinit::binary_path("8infinit").c_str(),
      arguments
  );
}



//
// ---------- slots ------------------------------------------------------------
//

void InfinitNetwork::_on_process_started()
{
  std::string pid_file = elle::os::path::join(_network_dir, "run.pid").c_str();
  LOG("Process successfully started (pid = %s)", this->_process.pid());
    {
      std::ofstream out(pid_file);
      if (out.good())
        out << this->_process.pid();
    }
}

void InfinitNetwork::_on_process_error(QProcess::ProcessError)
{
  ELLE_ERR("Process has an error.");
    {
      auto stdout = this->_process.readAllStandardOutput();
      auto stderr = this->_process.readAllStandardError();
      std::cerr << "=================================== standard output:\n"
                << QString(stdout).toStdString()
                << "\n=================================== standard error:\n"
                << QString(stderr).toStdString()
                ;

    }
}

void InfinitNetwork::_on_process_finished(int exit_code, QProcess::ExitStatus)
{
  ELLE_ERR("Process finished with exit code %s", exit_code);
  if (exit_code != 0)
    {
      auto stdout = this->_process.readAllStandardOutput();
      auto stderr = this->_process.readAllStandardError();
      std::cerr << "=================================== standard output:\n"
                << QString(stdout).toStdString()
                << "\n=================================== standard error:\n"
                << QString(stderr).toStdString()
                ;

    }
}
