#include "Module.hpp"
#include "Serialization.hpp"
#include <gtirb/Block.hpp>
#include <gtirb/CFG.hpp>
#include <gtirb/DataObject.hpp>
#include <gtirb/ImageByteMap.hpp>
#include <gtirb/Section.hpp>
#include <gtirb/Symbol.hpp>
#include <gtirb/SymbolicExpression.hpp>
#include <proto/Module.pb.h>
#include <gsl/gsl>
#include <map>

using namespace gtirb;

Module::Module(Context& C)
    : Node(Kind::Module), ImageBytes(ImageByteMap::Create(C)) {}

gtirb::SymbolSet& Module::getSymbols() { return *this->Symbols; }

const gtirb::SymbolSet& Module::getSymbols() const { return this->Symbols; }

gtirb::ImageByteMap& Module::getImageByteMap() {
  return *this->ImageBytes;
}

const gtirb::ImageByteMap& Module::getImageByteMap() const {
  return *this->ImageBytes;
}

const CFG& Module::getCFG() const { return *this->Cfg; }

CFG& Module::getCFG() { return this->Cfg; }

const DataSet& Module::getData() const { return this->Data; }

DataSet& Module::getData() { return this->Data; }

SectionSet& Module::getSections() { return this->Sections; }

const SectionSet& Module::getSections() const {
  return this->Sections;
}

SymbolicExpressionSet& Module::getSymbolicExpressions() {
  return this->SymbolicOperands;
}

const SymbolicExpressionSet& Module::getSymbolicExpressions() const {
  return this->SymbolicOperands;
}

void Module::toProtobuf(MessageType* Message) const {
  nodeUUIDToBytes(this, *Message->mutable_uuid());
  Message->set_binary_path(this->BinaryPath);
  Message->set_preferred_addr(static_cast<uint64_t>(this->PreferredAddr));
  Message->set_rebase_delta(this->RebaseDelta);
  Message->set_file_format(static_cast<proto::FileFormat>(this->FileFormat));
  Message->set_isa_id(static_cast<proto::ISAID>(this->IsaID));
  Message->set_name(this->Name);
  this->ImageBytes->toProtobuf(Message->mutable_image_byte_map());
  *Message->mutable_cfg() = gtirb::toProtobuf(this->Cfg);
  containerToProtobuf(this->Data, Message->mutable_data());
  containerToProtobuf(this->Sections, Message->mutable_sections());
  containerToProtobuf(this->SymbolicOperands,
                      Message->mutable_symbolic_operands());

  // Special case for symbol set: uses a multimap internally, serialized as a
  // repeated field.
  auto M = Message->mutable_symbols();
  initContainer(M, this->Symbols.size());
  std::for_each(
      this->Symbols.begin(), this->Symbols.end(),
      [M](const auto& N) { addElement(M, gtirb::toProtobuf(*N.second)); });
}

Module *Module::fromProtobuf(Context &C, const MessageType& Message) {
  Module *M = Module::Create(C);
  setNodeUUIDFromBytes(M, Message.uuid());
  M->BinaryPath = Message.binary_path();
  M->PreferredAddr = Addr(Message.preferred_addr());
  M->RebaseDelta = Message.rebase_delta();
  M->FileFormat = static_cast<gtirb::FileFormat>(Message.file_format());
  M->IsaID = static_cast<ISAID>(Message.isa_id());
  M->Name = Message.name();
  M->ImageBytes = ImageByteMap::fromProtobuf(C, Message.image_byte_map());
  gtirb::fromProtobuf(C, M->Cfg, Message.cfg());
  containerFromProtobuf(C, M->Data, Message.data());
  containerFromProtobuf(C, M->Sections, Message.sections());
  containerFromProtobuf(C, M->SymbolicOperands, Message.symbolic_operands());

  // Special case for symbol set: serialized as a repeated field, uses a
  // multimap internally.
  M->Symbols.clear();
  const auto& Syms = Message.symbols();
  std::for_each(Syms.begin(), Syms.end(), [M, &C](const auto& Elt) {
    addSymbol(M->Symbols, Symbol::fromProtobuf(C, Elt));
  });
  return M;
}
