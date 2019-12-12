//===- Module.cpp -----------------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2018 GrammaTech, Inc.
//
//  This code is licensed under the MIT license. See the LICENSE file in the
//  project root for license terms.
//
//  This project is sponsored by the Office of Naval Research, One Liberty
//  Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
//  N68335-17-C-0700.  The content of the information does not necessarily
//  reflect the position or policy of the Government and no official
//  endorsement should be inferred.
//
//===----------------------------------------------------------------------===//
#include "Module.hpp"
#include <gtirb/CFG.hpp>
#include <gtirb/CodeBlock.hpp>
#include <gtirb/Serialization.hpp>
#include <gtirb/SymbolicExpression.hpp>
#include <proto/Module.pb.h>
#include <map>

using namespace gtirb;

void Module::toProtobuf(MessageType* Message) const {
  nodeUUIDToBytes(this, *Message->mutable_uuid());
  Message->set_binary_path(this->BinaryPath);
  Message->set_preferred_addr(static_cast<uint64_t>(this->PreferredAddr));
  Message->set_rebase_delta(this->RebaseDelta);
  Message->set_file_format(static_cast<proto::FileFormat>(this->FileFormat));
  Message->set_isa_id(static_cast<proto::ISAID>(this->IsaID));
  Message->set_name(this->Name);
  *Message->mutable_cfg() = gtirb::toProtobuf(this->Cfg);
  sequenceToProtobuf(ProxyBlocks.begin(), ProxyBlocks.end(),
                     Message->mutable_proxies());
  sequenceToProtobuf(section_begin(), section_end(),
                     Message->mutable_sections());
  containerToProtobuf(Symbols, Message->mutable_symbols());
  if (EntryPoint) {
    nodeUUIDToBytes(EntryPoint, *Message->mutable_entry_point());
  } else {
    Message->clear_entry_point();
  }
  AuxDataContainer::toProtobuf(Message);
}

// FIXME: improve containerFromProtobuf so it can handle a pair where one
// element is a pointer to a Node subclass.
template <class T, class U, class V, class W>
static void nodeMapFromProtobuf(Context& C, std::map<T, U*>& Values,
                                const google::protobuf::Map<V, W>& Message) {
  Values.clear();
  std::for_each(Message.begin(), Message.end(), [&Values, &C](const auto& M) {
    std::pair<T, U*> Val;
    fromProtobuf(C, Val.first, M.first);
    Val.second = U::fromProtobuf(C, M.second);
    Values.insert(std::move(Val));
  });
}

Module* Module::fromProtobuf(Context& C, IR* Parent,
                             const MessageType& Message) {
  Module* M = Module::Create(C, Parent);
  setNodeUUIDFromBytes(M, Message.uuid());
  M->BinaryPath = Message.binary_path();
  M->PreferredAddr = Addr(Message.preferred_addr());
  M->RebaseDelta = Message.rebase_delta();
  M->FileFormat = static_cast<gtirb::FileFormat>(Message.file_format());
  M->IsaID = static_cast<ISAID>(Message.isa_id());
  M->Name = Message.name();
  for (const auto& Elt : Message.proxies())
    M->moveProxyBlock(ProxyBlock::fromProtobuf(C, M, Elt));
  for (const auto& Elt : Message.sections())
    M->moveSection(Section::fromProtobuf(C, M, Elt));
  for (const auto& Elt : Message.symbols())
    M->moveSymbol(Symbol::fromProtobuf(C, M, Elt));
  gtirb::fromProtobuf(C, M->Cfg, Message.cfg());
  if (!Message.entry_point().empty()) {
    M->EntryPoint = cast<CodeBlock>(
        Node::getByUUID(C, uuidFromBytes(Message.entry_point())));
  }
  static_cast<AuxDataContainer*>(M)->fromProtobuf(C, Message);
  return M;
}

template <typename NodeType, typename CollectionType>
static void modifyIndex(CollectionType& Index, NodeType* N,
                        const std::function<void()>& F) {
  Index.modify(Index.find(N), [&F](const auto&) { F(); });
}

void gtirb::addToModuleIndices(Node* N) {
  switch (N->getKind()) {
  case Node::Kind::ByteInterval: {
    auto BI = cast<ByteInterval>(N);
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      M->ByteIntervals.insert(BI);
      for (auto& B : BI->code_blocks()) {
        M->CodeBlocks.insert(cast<CodeBlock>(B.getNode()));
      }
      for (auto& B : BI->data_blocks()) {
        M->DataBlocks.insert(cast<DataBlock>(B.getNode()));
      }
      for (auto& SE : BI->symbolic_expressions()) {
        M->SymbolicExpressions.emplace(BI, SE.first);
      }
    }
  } break;
  case Node::Kind::CodeBlock: {
    auto B = cast<CodeBlock>(N);
    auto BI = B->getByteInterval();
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      M->CodeBlocks.insert(B);
    }
  } break;
  case Node::Kind::DataBlock: {
    auto B = cast<DataBlock>(N);
    auto BI = B->getByteInterval();
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      M->DataBlocks.insert(B);
    }
  } break;
  case Node::Kind::Section: {
    auto S = cast<Section>(N);
    auto M = S->getModule();
    if (M) {
      M->Sections.insert(S);
      for (auto BI : S->byte_intervals()) {
        addToModuleIndices(BI);
      }
    }
  } break;
  case Node::Kind::Symbol: {
    auto S = cast<Symbol>(N);
    auto M = S->getModule();
    if (M) {
      M->Symbols.insert(S);
    }
  } break;
  default: {
    throw std::runtime_error(
        "unexpected kind of node passed to addToModuleIndices!");
  }
  }
}

void gtirb::mutateModuleIndices(Node* N, const std::function<void()>& F) {
  switch (N->getKind()) {
  case Node::Kind::ByteInterval: {
    auto BI = cast<ByteInterval>(N);
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      auto& seIndex = M->SymbolicExpressions.get<Module::by_pointer>();
      for (auto& SE : BI->symbolic_expressions()) {
        seIndex.erase(seIndex.find(std::make_pair(BI, SE.first)));
      }

      modifyIndex(M->Sections.get<Module::by_pointer>(), S, [&]() {
        modifyIndex(M->ByteIntervals.get<Module::by_pointer>(), BI, F);
      });

      for (auto& SE : BI->symbolic_expressions()) {
        M->SymbolicExpressions.emplace(BI, SE.first);
      }
    } else {
      F();
    }
  } break;
  case Node::Kind::CodeBlock: {
    auto B = cast<CodeBlock>(N);
    auto BI = B->getByteInterval();
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      modifyIndex(M->Sections.get<Module::by_pointer>(), S, [&]() {
        modifyIndex(M->ByteIntervals.get<Module::by_pointer>(), BI, [&]() {
          modifyIndex(M->CodeBlocks.get<Module::by_pointer>(), B, F);
        });
      });
    } else {
      F();
    }
  } break;
  case Node::Kind::DataBlock: {
    auto B = cast<DataBlock>(N);
    auto BI = B->getByteInterval();
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      modifyIndex(M->Sections.get<Module::by_pointer>(), S, [&]() {
        modifyIndex(M->ByteIntervals.get<Module::by_pointer>(), BI, [&]() {
          modifyIndex(M->DataBlocks.get<Module::by_pointer>(), B, F);
        });
      });
    } else {
      F();
    }
  } break;
  case Node::Kind::Section: {
    auto S = cast<Section>(N);
    auto M = S->getModule();
    if (M) {
      modifyIndex(M->Sections.get<Module::by_pointer>(), S, F);
    } else {
      F();
    }
  } break;
  case Node::Kind::Symbol: {
    auto S = cast<Symbol>(N);
    auto M = S->getModule();
    if (M) {
      modifyIndex(M->Symbols.get<Module::by_pointer>(), S, F);
    } else {
      F();
    }
  } break;
  default: {
    throw std::runtime_error(
        "unexpected kind of node passed to mutateModuleIndices!");
  }
  }
}

void gtirb::removeFromModuleIndices(Node* N) {
  switch (N->getKind()) {
  case Node::Kind::ByteInterval: {
    auto BI = cast<ByteInterval>(N);
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      auto& index = M->ByteIntervals.get<Module::by_pointer>();
      index.erase(index.find(BI));

      auto& codeIndex = M->CodeBlocks.get<Module::by_pointer>();
      for (auto& B : BI->code_blocks()) {
        codeIndex.erase(codeIndex.find(cast<CodeBlock>(B.getNode())));
      }

      auto& dataIndex = M->DataBlocks.get<Module::by_pointer>();
      for (auto& B : BI->data_blocks()) {
        dataIndex.erase(dataIndex.find(cast<DataBlock>(B.getNode())));
      }

      auto& seIndex = M->SymbolicExpressions.get<Module::by_pointer>();
      for (auto& SE : BI->symbolic_expressions()) {
        seIndex.erase(seIndex.find(std::make_pair(BI, SE.first)));
      }
    }
  } break;
  case Node::Kind::CodeBlock: {
    auto B = cast<CodeBlock>(N);
    auto BI = B->getByteInterval();
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      auto& index = M->CodeBlocks.get<Module::by_pointer>();
      index.erase(index.find(B));
    }
  } break;
  case Node::Kind::DataBlock: {
    auto B = cast<DataBlock>(N);
    auto BI = B->getByteInterval();
    auto S = BI ? BI->getSection() : nullptr;
    auto M = S ? S->getModule() : nullptr;
    if (M) {
      auto& index = M->DataBlocks.get<Module::by_pointer>();
      index.erase(index.find(B));
    }
  } break;
  case Node::Kind::Section: {
    auto S = cast<Section>(N);
    auto M = S->getModule();
    if (M) {
      auto& index = M->Sections.get<Module::by_pointer>();
      index.erase(index.find(S));
      for (auto BI : S->byte_intervals()) {
        removeFromModuleIndices(BI);
      }
    }
  } break;
  case Node::Kind::Symbol: {
    auto S = cast<Symbol>(N);
    auto M = S->getModule();
    if (M) {
      auto& index = M->Symbols.get<Module::by_pointer>();
      index.erase(index.find(S));
    }
  } break;
  default: {
    throw std::runtime_error(
        "unexpected kind of node passed to mutateModuleIndices!");
  }
  }
}
