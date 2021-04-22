#ifndef BTREE_H__
#define BTREE_H__

#include<array>
#include<vector>
#include<string>
#include<cstdint>
#include<utility>

#include "memory.h"

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

enum NodeType : ::uint8_t {
  kNodeInternal,
  kNodeLeaf,
};

struct NodeHeader {
  NodeType    nodeType_;
  ::uint8_t   isRoot_;
  ::uint32_t  parentPointer_;
};

struct LeafNodeHeader {
  ::uint32_t cellsCount_;
};

constexpr ::uint32_t kMaxCells =  \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(LeafNodeHeader))
  / sizeof(Pair<::uint32_t, memory::Row>);

using Cell = Pair<::uint32_t, memory::Row>;

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
