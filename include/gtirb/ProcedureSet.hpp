#pragma once

#include <gtirb/EA.hpp>
#include <gtirb/Node.hpp>
#include <gtirb/Procedure.hpp>
#include <map>
#include <memory>

namespace gtirb
{
    class Procedure;

    ///
    /// \class ProcedureSet
    /// \author John E. Farrier
    ///
    /// Storage for all gtirb::Procedure objects for a single gtirb::Module.
    ///
    class GTIRB_GTIRB_EXPORT_API ProcedureSet : public Node
    {
    public:
        ///
        /// Default constructor.
        ///
        ProcedureSet();

        ///
        /// Defaulted trivial destructor.
        ///
        ~ProcedureSet() override;

        ///
        /// Get the Procedure at the given EA.
        ///
        /// \param x    The EA of the gtirb::Procedure to get.
        /// \return     The Procedure at the given EA or nullptr.
        ///
        Procedure* getProcedure(gtirb::EA x);
        const Procedure* getProcedure(gtirb::EA x) const;

        ///
        /// Create the Procedure at the given EA.
        ///
        /// This is preferable to adding Procedures manually as it ensures no duplicate Procedures
        /// are
        /// created.
        ///
        /// \pre No procedure currently exists at the given EA.
        ///
        /// \param x    The EA of the gtirb::Procedure to get (or create).
        /// \return     A reference to Procedure at the given EA.
        ///
        Procedure& createProcedure(gtirb::EA x);

        ///
        /// Serialization support.
        ///
        template <class Archive>
        void serialize(Archive& ar, const unsigned int /*version*/);

    private:
        std::map<EA, Procedure> contents;
    };
} // namespace gtirb

BOOST_CLASS_EXPORT_KEY(gtirb::ProcedureSet);
