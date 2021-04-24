#pragma once

#include "def.h"
#include "memory.h"

#include<array>
#include<vector>
#include<string>
#include<cstdint>
#include<utility>

namespace structure {

#pragma pack(push)
#pragma pack(1)

// Memory is strictly aligned compared to std::pair
template<typename T, typename U>
struct Pair {
  T key_;
  U value_;

  Pair() = default;
  Pair(T key, U value): key_(key), value_(value) {}
};

template<typename T, typename U>
Pair(T, U) -> Pair<T, U>;

enum NodeType : u8 {
  kNodeInternal,
  kNodeLeaf,
};

struct NodeHeader {
  NodeType  nodeType_;
  u8        isRoot_;
  u32       parentPointer_;
};

struct LeafNodeHeader {
  u32 cellsCount_;
};

struct InternalNodeHeader {
  u32 childCount_;
  Addr rightestChild_;
};

using Cell = Pair<u32, memory::Row>;

using Child = Pair<u32, Addr>;

constexpr u32 kMaxCells =  \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(LeafNodeHeader)) / sizeof(Cell);

constexpr u32 kMaxChild = \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(InternalNodeHeader)) / sizeof(Child);

struct LeafNodeBody {
  std::array<Cell, kMaxCells> cells;
};

struct InternalNodeBody {
  std::array<Child, kMaxChild> childs;
};

struct alignas(memory::kPageSize) LeafNode {
  NodeHeader      nodeHeader_;
  LeafNodeHeader  leafNodeHeader_;
  LeafNodeBody    leafNodeBody_;
};

struct alignas(memory::kPageSize) InternalNode {
  NodeHeader          nodeHeader_;
  InternalNodeHeader  internalNodeHeader_;
  InternalNodeBody    internalNodeBody_;
};

#pragma pack(pop)

// template<typename Node, u32 kMaxNodes>
// class BTree {
//  private:
//   std::array<Node, kMaxNodes> nodes;

//  public:
//   void split() {
    
//   }
// };

}   // namespace structure ends
