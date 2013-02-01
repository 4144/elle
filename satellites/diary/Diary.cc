#include <limits>

#include <elle/Elle.hh>
#include <elle/utility/Parser.hh>

#include <elle/concept/Fileable.hh>

#include "Diary.hh"
#include "Memoirs.hh"
#include <satellites/diary/Diary.hh>


namespace satellite
{

//
// ---------- definitions -----------------------------------------------------
//

//
// ---------- functions -------------------------------------------------------
//

  ///
  /// the main function.
  ///
  elle::Status          Main(elle::Natural32                    argc,
                             elle::Character*                   argv[])
  {
    Diary::Operation    operation;

    // initialize the Elle library.
    if (elle::Elle::Initialize() == elle::Status::Error)
      throw elle::Exception("unable to initialize Elle");

    // set up the program.
    if (elle::concurrency::Program::Setup() == elle::Status::Error)
      throw elle::Exception("unable to set up the program");

    // initialize the Lune library.
    if (lune::Lune::Initialize() == elle::Status::Error)
      throw elle::Exception("unable to initialize Lune");

    // initialize Infinit.
    if (Infinit::Initialize() == elle::Status::Error)
      throw elle::Exception("unable to initialize Infinit");

    // initialize the operation.
    operation = Diary::OperationUnknown;

    // allocate a new parser.
    Infinit::Parser = new elle::Parser(argc, argv);

    // specify a program description.
    if (Infinit::Parser->Description(Infinit::Copyright) == elle::Status::Error)
      throw elle::Exception("unable to set the description");

    // register the options.
    if (Infinit::Parser->Register(
          "Help",
          'h',
          "help",
          "display the help",
          elle::Parser::KindNone) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Record",
          'c',
          "record",
          "activate the event recording",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Replay",
          'y',
          "replay",
          "activate the event replaying",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Dump",
          'd',
          "dump",
          "activate the dumping of the given diary",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Mountpoint",
          'm',
          "mounpoint",
          "specify the path to the file system mounpoint",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "Mirror",
          'i',
          "mirror",
          "specify the path to the directory which must be mirrored through "
          "the mounpoint",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "From",
          'f',
          "from",
          "specify the number of the first operation to be triggered from "
          "the diary",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the option.
    if (Infinit::Parser->Register(
          "Network",
          'n',
          "network",
          "specifies the name of the network",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // register the options.
    if (Infinit::Parser->Register(
          "To",
          't',
          "to",
          "specify the number of the last operation to be triggered from "
          "the diary",
          elle::Parser::KindRequired) == elle::Status::Error)
      throw elle::Exception("unable to register the option");

    // add an example.
    if (Infinit::Parser->Example(
          "-c test.dia -m ~/local/mnt/test/ -i /tmp/test") ==
        elle::Status::Error)
      throw elle::Exception("unable to set an example");

    // add an example.
    if (Infinit::Parser->Example(
          "-y test.dia -n mynetwork") ==
        elle::Status::Error)
      throw elle::Exception("unable to set an example");

    // parse.
    if (Infinit::Parser->Parse() == elle::Status::Error)
      throw elle::Exception("unable to parse the command line");

    // test the option.
    if (Infinit::Parser->Test("Help") == true)
      {
        // display the usage.
        Infinit::Parser->Usage();

        // quit.
        return elle::Status::Ok;
      }

    // initialize the Etoile library.
    if (etoile::Etoile::Initialize() == elle::Status::Error)
      throw elle::Exception("unable to initialize Etoile");

    // check the mutually exclusive options.
    if ((Infinit::Parser->Test("Record") == true) &&
        (Infinit::Parser->Test("Replay") == true) &&
        (Infinit::Parser->Test("Dump") == true))
      throw elle::Exception("the record and replay options are mutually exclusive");

    // test the option.
    if (Infinit::Parser->Test("Record") == true)
      operation = Diary::OperationRecord;

    // test the option.
    if (Infinit::Parser->Test("Replay") == true)
      operation = Diary::OperationReplay;

    // test the option.
    if (Infinit::Parser->Test("Dump") == true)
      operation = Diary::OperationDump;

    // trigger a command.
    switch (operation)
      {
      case Diary::OperationRecord:
        {
          elle::String          mountpoint;
          elle::String          mirror;
          elle::String          string;
          elle::Path            path;

          // retrieve the string-based path.
          if (Infinit::Parser->Value("Record",
                                     string) == elle::Status::Error)
            throw elle::Exception("unable to retrieve the path value");

          // create the path.
          if (path.Create(string) == elle::Status::Error)
            throw elle::Exception("unable to create the path");

          // retrieve the mountpoint.
          if (Infinit::Parser->Value("Mountpoint",
                                     mountpoint) == elle::Status::Error)
            throw elle::Exception("unable to retrieve the mountpoint value");

          // retrieve the mirror.
          if (Infinit::Parser->Value("Mirror",
                                     mirror) == elle::Status::Error)
            throw elle::Exception("unable to retrieve the mirror value");

#if defined(INFINIT_LINUX) || defined(INFINIT_MACOSX)
          {
            unix::Memoirs       memoirs;

            // initialize the memoirs.
            if (memoirs.Initialize(mountpoint, mirror) == elle::Status::Error)
              throw elle::Exception("unable to initialize the memoirs");

            // launch the program.
            elle::Program::Launch();

            // clean the memoirs.
            if (memoirs.Clean() == elle::Status::Error)
              throw elle::Exception("unable to clean the memoirs");

            // store the memoirs.
            if (memoirs.Store(path) == elle::Status::Error)
              throw elle::Exception("unable to store the memoirs");
          }
#elif defined(INFINIT_WINDOWS)
          {
            // XXX todo: windows
          }
#else
# error "unsupported platform"
#endif

          // display a message.
          std::cout << "The sequence of file system operations have been "
                    << "successfully recorded in '" << path.str() << "'!"
                    << std::endl;

          break;
        }
      case Diary::OperationReplay:
        {
          elle::String          string;
          elle::Path            path;
          elle::Natural32       from;
          elle::Natural32       to;

          // retrieve the string-based path.
          if (Infinit::Parser->Value("Replay",
                                     string) == elle::Status::Error)
            throw elle::Exception("unable to retrieve the path value");

          // create the path.
          if (path.Create(string) == elle::Status::Error)
            throw elle::Exception("unable to create the path");

          // initialize the indexes.
          from = 0;
          to = std::numeric_limits<decltype(to)>::max();

          // retrieve the from.
          if ((Infinit::Parser->Test("From") == true) &&
              (Infinit::Parser->Value(
                 "From",
                 from) == elle::Status::Error))
              throw elle::Exception("unable to retrieve the from value");

          // retrieve the to.
          if ((Infinit::Parser->Test("To") == true) &&
              (Infinit::Parser->Value(
                 "To",
                 to) == elle::Status::Error))
            throw elle::Exception("unable to retrieve the to value");

          // retrieve the network name.
          if (Infinit::Parser->Value("Network",
                                     Infinit::Network) == elle::Status::Error)
            throw elle::Exception("unable to retrieve the network name");

          // initialize the Hole library.
          hole::Hole::Initialize();

          // initialize the Agent library.
          if (agent::Agent::Initialize() == elle::Status::Error)
            throw elle::Exception("unable to initialize Agent");

#if defined(INFINIT_LINUX) || defined(INFINIT_MACOSX)
          {
            unix::Memoirs       memoirs;

            // load the memoirs.
            if (memoirs.Load(path) == elle::Status::Error)
              throw elle::Exception("unable to load the memoirs");

            // initialize the memoirs.
            if (memoirs.Initialize(from, to) == elle::Status::Error)
              throw elle::Exception("unable to initialize the memoirs");

            // launch the program.
            elle::Program::Launch();

            // clean the memoirs.
            if (memoirs.Clean() == elle::Status::Error)
              throw elle::Exception("unable to clean the memoirs");
          }
#elif defined(INFINIT_WINDOWS)
          {
            // XXX todo: windows
          }
#else
# error "unsupported platform"
#endif

          // clean the Agent library.
          if (agent::Agent::Clean() == elle::Status::Error)
            throw elle::Exception("unable to clean Agent");

          // clean Hole.
          if (hole::Hole::Clean() == elle::Status::Error)
            throw elle::Exception("unable to clean Hole");

          break;
        }
      case Diary::OperationDump:
        {
          elle::String          string;
          elle::Path            path;

          // retrieve the string-based path.
          if (Infinit::Parser->Value("Dump",
                                     string) == elle::Status::Error)
            throw elle::Exception("unable to retrieve the path value");

          // create the path.
          if (path.Create(string) == elle::Status::Error)
            throw elle::Exception("unable to create the path");

#if defined(INFINIT_LINUX) || defined(INFINIT_MACOSX)
          {
            unix::Memoirs       memoirs;

            // load the memoirs.
            if (memoirs.Load(path) == elle::Status::Error)
              throw elle::Exception("unable to load the memoirs");

            // dump the memoirs.
            if (memoirs.Dump() == elle::Status::Error)
              throw elle::Exception("unable to dump the memoirs");
          }
#elif defined(INFINIT_WINDOWS)
          {
            // XXX todo: windows
          }
#else
# error "unsupported platform"
#endif

          break;
        }
      case Diary::OperationUnknown:
      default:
        {
          // display the usage.
          Infinit::Parser->Usage();

          throw elle::Exception("please specify an operation to perform");
        }
      }

    // delete the parser.
    delete Infinit::Parser;

    // clean the Etoile.
    if (etoile::Etoile::Clean() == elle::Status::Error)
      throw elle::Exception("unable to clean Etoile");

    // clean Infinit.
    if (Infinit::Clean() == elle::Status::Error)
      throw elle::Exception("unable to clean Infinit");

    // clean Lune
    if (lune::Lune::Clean() == elle::Status::Error)
      throw elle::Exception("unable to clean Lune");

    // clean Elle.
    if (elle::Elle::Clean() == elle::Status::Error)
      throw elle::Exception("unable to clean Elle");

    return elle::Status::Ok;
  }

}

//
// ---------- main ------------------------------------------------------------
//

///
/// this is the program entry point.
///
int                     main(int                                argc,
                             char**                             argv)
{
  try
    {
      if (satellite::Main(argc, argv) == elle::Status::Error)
        {
          show();

          return (1);
        }
    }
  catch (std::exception& e)
    {
      std::cout << "The program has been terminated following "
                << "a fatal error (" << e.what() << ")." << std::endl;

      return (1);
    }

  return (0);
}
