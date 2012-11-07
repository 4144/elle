#include <package/module/CodingStandard.hh>

#include <system-dependecy.h>
#include <library-dependency.h>

/*
 * The namespaces are re-specified.
 *
 * Note that the only allowed 'using namespace' are in the implementation
 * file for its own module, or in a function scope when it improves
 * readability:
 *
 * void f()
 * {
 *    namespace tcp = boost::asio::tcp;
 *    tcp::socket s(io_service);
 * }
 *
 */
namespace package
{
  namespace module
  {

    /*------------.
    | Definitions |
    `------------*/

    Natural32 CodingStandard::average_size = 42;

    /*---------------.
    | Static Methods |
    `---------------*/

    CodingStandard*
    CodingStandard::generate()
    {
      /*
       * Pointers must always be initialized with 'nullptr'. Note that
       * the old C macro NULL must never be used.
       */
      CodingStandard* coding_standard = nullptr;

      /*
       * Note that braces {} should be used for object construction because
       * the standard is ambiguous in some cases with parentheses, as shown
       * below. The following example does not compile because the compiler
       * thinks a inner function named 'plain' is defined, returning a Plain
       * and taking an argument whose name is named 'buffer' and of type
       * WeakBuffer.
       *
       *   cryptography::Plain plain(elle::WeakBuffer(buffer));
       *
       * Using braces makes it easy for the compiler to distinguish object
       * construction (classes and structures) from function calls.
       */
      coding_standard = new CodingStandard{1, 5};

      /*
       * Every block of logically related operations must be commented so
       * as to explain what is being done.
       */
      // Comment describing the following operations.

      /*
       * Note that the identation is always composed of two characters
       * even for the brackets following a for (...) or if (...):
       *
       *   for (...)
       *   >>{
       *   >>>>int i;
       *   >>}
       */
      for (; iterator != end; ++iterator)
        {
          // Something important is done here.
        }

      return (coding_standard);
    }

    /*-------------.
    | Construction |
    `-------------*/

    CodingStandard::CodingStandard():
      /*
       * The constructor must be directly followed by a colon, without a space
       * in between.
       *
       * Additionally, every attribute must be initialized on its own line,
       * as shown below.
       */
      _something(1.1),
      _size(2)
    {
    }

    CodingStandard::~CodingStandard()
    {
    }

    /*--------.
    | Methods |
    `--------*/

    /*
     * Every type must be specified using its complete namespace hierarchy.
     * As a corollary, namespace should be kept short.
     */
    Natural32
    CodingStandard::compute(another::place::SomeOtherClass const& something,
                            Natural32 const somethingelse)
    {
      /*
       * Note that every call to a static method, even if local to the
       * current class, should be prefixed by the name of the class,
       * as shown below.
       */
      CodingStandard coding_standard = CodingStandard::generate();

      /*
       * Sometimes, it is more readable to put every argument
       * of a method call on its own line rather than having an
       * extremely long call.
       */
      return (something.encrypt(somethingelse,
                                static_cast<Natural64>(this->_size)));
    }

    Real
    _inner_action(String const& left,
                  Natural32 const right)
    {
      /*
       * One must declare one local variable per line and never use
       * delcarations such as:
       *
       *   auto iterator = list.begin(),
       *        end = list.end();
       *
       * where it is difficult to know if two variables are being declared
       * or a single one with a complicated initialization expression.
       */
      Natural32 length;
      Natural32 i;

      return (3.14);
    }

  }
}
