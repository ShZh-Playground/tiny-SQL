#ifndef BTREE_H__
#define BTREE_H__

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
template<typename T1, typename T2>
struct Pair {
  T1 key_;
  T2 value_;

  Pair() = default;
  Pair(T1 key, T2 value): key_(key), value_(value) {}
};

template<typename T1, typename T2>
Pair(T1, T2) -> Pair<T1, T2>;

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

constexpr u32 kMaxCells =  \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(LeafNodeHeader))
  / sizeof(Pair<u32, memory::Row>);

using Cell = Pair<u32, memory::Row>;

struct LeafNodeBody {
  std::array<Cell, kMaxCells> cells;
};

struct alignas(memory::kPageSize) LeafNode {
  NodeHeader      nodeHeader_;
  LeafNodeHeader  leafNodeHeader_;
  LeafNodeBody    leafNodeBody_;
};

#pragma pack(pop)

// template<typename T>
// class BTree {

// };

}   // namespace structure ends

#endif  // BTREE_H__ ends
