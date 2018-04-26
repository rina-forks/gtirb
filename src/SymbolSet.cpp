#include <gtirb/Module.hpp>
#include <gtirb/NodeValidators.hpp>
#include <gtirb/Symbol.hpp>
#include <gtirb/SymbolSet.hpp>

using namespace gtirb;

BOOST_CLASS_EXPORT_IMPLEMENT(gtirb::SymbolSet);

SymbolSet::SymbolSet() : Node()
{
    this->addParentValidator(gtirb::NodeValidatorHasParentOfType<gtirb::Module>());
    this->addParentValidator(NodeValidatorHasNoSiblingsOfType<gtirb::SymbolSet>());
}

Symbol* SymbolSet::getSymbol(gtirb::EA x) const
{
    /// \todo   Implement a recursive find function for Nodes.

    // This grabs all symbols every time, which could be slow (measure first).
    const auto symbols = GetChildrenOfType<Symbol>(this, true);
    const auto found = std::find_if(std::begin(symbols), std::end(symbols),
                                    [x](Symbol* s) { return s->getEA() == x; });

    if(found != std::end(symbols))
    {
        return *found;
    }

    return nullptr;
}

Symbol* SymbolSet::getOrCreateSymbol(gtirb::EA x)
{
    auto symbol = this->getSymbol(x);

    if(symbol == nullptr)
    {
        auto newSymbol = std::make_unique<Symbol>(x);
        symbol = newSymbol.get();
        this->push_back(std::move(newSymbol));
    }

    return symbol;
}
