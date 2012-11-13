#include "metalib.hh"

#include <elle/io/Path.hh>
#include <elle/types.hh>

#include <cryptography/random.hh>
// XXX[temporary: for cryptography]
using namespace infinit;

#include <elle/Passport.hh>
#include <elle/Authority.hh>

// XXX When Qt is out, remove this
#ifdef slots
# undef slots
#endif
#ifdef slot
# undef slot
#endif

#include "passport.hh"

//
// ---------- functions  ------------------------------------------------------
//

static elle::Passport create_passport(elle::String const& id,
                                      elle::String const& authority_file,
                                      elle::String const& authority_password)
{
  elle::io::Path      authority_path;

  if (authority_path.Create(authority_file) == elle::Status::Error)
    throw std::runtime_error("unable to create authority path");

  // Load the authority file.
  elle::Authority authority(authority_path);

  // decrypt the authority.
  if (authority.Decrypt(authority_password) == elle::Status::Error)
    throw std::runtime_error("unable to decrypt the authority");

  //
  // create the passport.
  //
  elle::Buffer buffer(
    cryptography::random::generate<elle::Buffer>(512));
  // XXX
  elle::standalone::Region region;
  if (region.Duplicate(buffer.contents(), buffer.size()) == elle::Status::Error)
    escape("XXX");

  // create a label.
  hole::Label label(region);

  // create the passport.
  elle::Passport passport(label, id);

  // seal the passport.
  if (passport.Seal(authority) == elle::Status::Error)
    throw std::runtime_error("unable to seal the passport");

  return passport;
}

extern "C" PyObject* metalib_generate_passport(PyObject*, PyObject* args)
{
  PyObject* ret = nullptr;
  char const* id = nullptr,
            * authority_file = nullptr,
            * authority_password = nullptr;
  if (!PyArg_ParseTuple(args, "sss:generate_passport",
                        &id, &authority_file, &authority_password))
    return nullptr;
  if (!id || !authority_file || !authority_password)
    return nullptr;

  Py_BEGIN_ALLOW_THREADS;

  try
    {
      auto passport = create_passport(id, authority_file, authority_password);
      elle::String passport_string;
      if (passport.Save(passport_string) != elle::Status::Error)
        {
          ret = Py_BuildValue("s", passport_string.c_str());
        }
      else
        {
          PyErr_SetString(
              metalib_MetaError,
              "Cannot convert the passport to string"
          );
        }
    }
  catch(std::exception const& err)
    {
      PyErr_SetString(metalib_MetaError, err.what());
    }

  Py_END_ALLOW_THREADS;

  return ret;
}
