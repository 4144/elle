#include <lune/Configuration.hh>

#include <elle/utility/Settings.hh>

#include <elle/io/Path.hh>
#include <elle/io/Piece.hh>

#include <lune/Lune.hh>

namespace lune
{

//
// ---------- definitions -----------------------------------------------------
//

  ///
  /// etoile-specific configuration values.
  ///
  const elle::Boolean
  Configuration::Default::Etoile::Debug = false;

  const elle::Natural32
  Configuration::Default::Etoile::Gear::Containment = 2000;

  const elle::Boolean
  Configuration::Default::Etoile::Shrub::Status = true;
  const elle::Natural32
  Configuration::Default::Etoile::Shrub::Capacity = 1024;
  const elle::Natural32
  Configuration::Default::Etoile::Shrub::Lifespan = 300;
  const elle::Natural32
  Configuration::Default::Etoile::Shrub::Frequency = 120000;

  const elle::Boolean
  Configuration::Default::Etoile::History::Status = false;
  const elle::Character
  Configuration::Default::Etoile::History::Indicator::Root = '@';
  const elle::Character
  Configuration::Default::Etoile::History::Indicator::Slab = '%';

  ///
  /// nucleus-specific configuration values.
  ///
  const elle::Boolean
  Configuration::Default::Nucleus::Debug = false;

  ///
  /// hole-specific configuration values.
  ///
  const elle::Boolean
  Configuration::Default::Hole::Debug = false;

  ///
  /// horizon-specific configuration values.
  ///
  const elle::Boolean
  Configuration::Default::Horizon::Debug = false;

  ///
  /// satellites-specific configuration values.
  ///
  const elle::Boolean
  Configuration::Default::Satellites::Debug = false;

//
// ---------- methods ---------------------------------------------------------
//

  ///
  /// this method synchronises the in-memory configuration so as to
  /// be stored.
  ///
  elle::Status          Configuration::Push()
  {
    //
    // etoile
    //
    (*this)["etoile"].Set("debug", this->etoile.debug);
    (*this)["etoile"].Set("gear.containment", this->etoile.gear.containment);
    (*this)["etoile"].Set("shrub.status", this->etoile.shrub.status);
    (*this)["etoile"].Set("shrub.capacity", this->etoile.shrub.capacity);
    (*this)["etoile"].Set("shrub.lifespan", this->etoile.shrub.lifespan);
    (*this)["etoile"].Set("shrub.frequency", this->etoile.shrub.frequency);
    (*this)["etoile"].Set("history.status", this->etoile.history.status);
    (*this)["etoile"].Set("history.indicator.root",
                          this->etoile.history.indicator.root);
    (*this)["etoile"].Set("history.indicator.slab",
                          this->etoile.history.indicator.slab);

    //
    // nucleus
    //
    (*this)["nucleus"].Set("debug", this->nucleus.debug);

    //
    // hole
    //
    (*this)["hole"].Set("debug", this->hole.debug);

    //
    // horizon
    //
    (*this)["horizon"].Set("debug", this->horizon.debug);

    //
    // satellites
    //
    (*this)["satellites"].Set("debug", this->satellites.debug);

    return elle::Status::Ok;
  }

  ///
  /// this method updates the in-memory parameters according to the
  /// associated settings.
  ///
  elle::Status          Configuration::Pull()
  {
    //
    // etoile
    //
    this->etoile.debug = (*this)["etoile"].Get("debug",
        Configuration::Default::Etoile::Debug);

    this->etoile.gear.containment = (*this)["etoile"].Get("gear.containment",
        Configuration::Default::Etoile::Gear::Containment);

    this->etoile.shrub.status = (*this)["etoile"].Get("shrub.status",
        Configuration::Default::Etoile::Shrub::Status);

    this->etoile.shrub.capacity = (*this)["etoile"].Get("shrub.capacity",
        Configuration::Default::Etoile::Shrub::Capacity);

    this->etoile.shrub.lifespan = (*this)["etoile"].Get("shrub.lifespan",
        Configuration::Default::Etoile::Shrub::Lifespan);

    this->etoile.shrub.frequency = (*this)["etoile"].Get("shrub.frequency",
        Configuration::Default::Etoile::Shrub::Frequency);

    this->etoile.history.status = (*this)["etoile"].Get("history.status",
        Configuration::Default::Etoile::History::Status);

    this->etoile.history.indicator.root =
      (*this)["etoile"].Get("history.indicator.root",
          Configuration::Default::Etoile::History::Indicator::Root);

    this->etoile.history.indicator.slab =
      (*this)["etoile"].Get("history.indicator.slab",
          Configuration::Default::Etoile::History::Indicator::Slab);

    //
    // nucleus
    //
    this->nucleus.debug = (*this)["nucleus"].Get("debug",
        Configuration::Default::Nucleus::Debug);

    //
    // hole
    //
    this->hole.debug = (*this)["hole"].Get("debug",
        Configuration::Default::Hole::Debug);

    //
    // horizon
    //
    this->horizon.debug = (*this)["horizon"].Get("debug",
        Configuration::Default::Horizon::Debug);

    //
    // satellites
    //
    this->satellites.debug = (*this)["satellites"].Get("debug",
        Configuration::Default::Satellites::Debug);

    return elle::Status::Ok;
  }

  elle::io::Path
  Configuration::_path(elle::String const& user)
  {
    return (elle::io::Path(Lune::Configuration,
                           elle::io::Piece("%USER%", user)));
  }

//
// ---------- dumpable --------------------------------------------------------
//

  ///
  /// this method dumps a configuration.
  ///
  /// note that this method may actually not dump the current values of
  /// the parameters.
  ///
  elle::Status          Configuration::Dump(const elle::Natural32 margin) const
  {
    elle::String        alignment(margin, ' ');

    std::cout << alignment << "[Configuration]" << std::endl;

    // dump the parent settings.
    if (elle::utility::Settings::Dump(margin + 2) == elle::Status::Error)
      throw elle::Exception("unable to dump the settings");

    return elle::Status::Ok;
  }

//
// ---------- fileable --------------------------------------------------------
//

  void
  Configuration::load(elle::String const& user)
  {
    this->load(Configuration::_path(user));
  }

  void
  Configuration::store(elle::String const& user) const
  {
    this->store(Configuration::_path(user));
  }

  void
  Configuration::erase(elle::String const& user)
  {
    elle::concept::Fileable<>::erase(Configuration::_path(user));
  }

  elle::Boolean
  Configuration::exists(elle::String const& user)
  {
    return (elle::concept::Fileable<>::exists(Configuration::_path(user)));
  }

}
