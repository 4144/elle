//
// ---------- header ----------------------------------------------------------
//
// project       elle
//
// license       infinit
//
// author        julien quintard   [fri nov 27 22:04:36 2009]
//

//
// ---------- includes --------------------------------------------------------
//

#include <elle/test/network/local/Server.hh>

namespace elle
{
  namespace test
  {

//
// ---------- methods ---------------------------------------------------------
//

    ///
    /// this method initializes the server.
    ///
    Status              Server::Setup(const String&             line)
    {
      // set the line.
      this->line = line;

      return Status::Ok;
    }

    ///
    /// this method is the thread entry point.
    ///
    Status              Server::Run()
    {
      std::cout << "[line] " << line << std::endl;

      // listen for incoming connections.
      if (LocalServer::Listen(this->line,
                              Callback<>::Infer(&Server::Connection,
                                                this)) == Status::Error)
        escape("unable to listen for local connections");

      return Status::Ok;
    }

//
// ---------- callbacks -------------------------------------------------------
//

    ///
    /// this method handles new connections.
    ///
    Status              Server::Connection(LocalSocket*         socket)
    {
      String            challenge("CHALLENGE");
      String            response;

      std::cout << "[challenging...] " << std::endl;

      // call the challenge.
      if (socket->Call(Inputs<TagChallenge>(challenge),
                       Outputs<TagResponse>(response)) == Status::Error)
        escape("unable to call the challenge");

      std::cout << "[response] " << response << std::endl;

      // check the response
      if (response != elle::String("RESPONSE"))
        std::cerr << "unexpected response '" << response << "'" << std::endl;

      elle::Program::Exit();

      return Status::Ok;
    }

  }
}
