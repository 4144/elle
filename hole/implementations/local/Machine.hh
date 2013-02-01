#ifndef HOLE_IMPLEMENTATIONS_LOCAL_MACHINE_HH
# define HOLE_IMPLEMENTATIONS_LOCAL_MACHINE_HH

# include <memory>

# include <elle/types.hh>

# include <hole/fwd.hh>

# include <nucleus/proton/fwd.hh>

namespace hole
{
  namespace implementations
  {
    namespace local
    {
      /// The current host.
      ///
      /// Note that every host can emulate multiple nodes though in
      /// the case of the local implementation, a single host is
      /// present i.e the machine.
      class Machine
      {
      public:
        Machine(hole::Hole& hole);
        /// Store an immutable block.
        void
        put(const nucleus::proton::Address&,
            const nucleus::proton::ImmutableBlock&);
        /// Store a mutable block.
        void
        put(const nucleus::proton::Address&,
            const nucleus::proton::MutableBlock&);
        /// Retrieve an immutable block.
        std::unique_ptr<nucleus::proton::Block>
        get(nucleus::proton::Address const&);
        /// Retrieve a mutable block.
        std::unique_ptr<nucleus::proton::Block>
        get(nucleus::proton::Address const&, nucleus::proton::Revision const&);
        /// Remove a block.
        void
        wipe(const nucleus::proton::Address&);

        //
        // interfaces
        //

        // dumpable
        elle::Status            Dump(const elle::Natural32 = 0) const;
      private:
        hole::Hole& _hole;
      };

    }
  }
}

#endif
