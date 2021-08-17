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

enum class NodeType : u8 {
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

  LeafNode(u32 parent_pointer) {
    this->nodeHeader_ = NodeHeader {
      .nodeType_ = NodeType::kNodeLeaf,
      .isRoot_ = false,
      .parentPointer_ = parent_pointer,
    };
    this->leafNodeHeader_ = LeafNodeHeader {
        .cellsCount_ = 0,
    };
    this->leafNodeBody_ = LeafNodeBody {
        .cells = std::array<Cell, kMaxCells>()
    };
  }
};

struct alignas(memory::kPageSize) InternalNode {
  NodeHeader          nodeHeader_;
  InternalNodeHeader  internalNodeHeader_;
  InternalNodeBody    internalNodeBody_;
};

#pragma pack(pop)

class BTree {
 public:
  StatusCode insert_leaf_node(memory::Table& table, memory::Cursor& cursor, u32 key, memory::Row row) {
    auto* page = table.pager_.getPage(cursor.getPageIndex());
    auto* node = reinterpret_cast<LeafNode*>(page);

    if (node->leafNodeHeader_.cellsCount_ >= kMaxCells) {
      std::cerr << "Cannot insert into a full node! Split operation is required!" << std::endl;
      return StatusCode::kInsertError;
    }
    // Page里面的各个cell依然是一个数组。
    // 插入的时候把相应位置后面的元素都往后移一位
    u32 insert_index = cursor.getCellIndex();
    for (u32 index = kMaxCells - 1; index > insert_index; --index) {
        ::memcpy(&node->leafNodeBody_.cells[index], &node->leafNodeBody_.cells[index - 1], sizeof(Cell));
    }

    ++node->leafNodeHeader_.cellsCount_;
    node->leafNodeBody_.cells[insert_index] = Pair(key, row);

    return StatusCode::kSuccess;
  }

  memory::Cursor find_key(memory::Table& table, u32 key) {
      auto* page = table.pager_.getPage(table.cursor_.getPageIndex());
      auto* node = reinterpret_cast<LeafNode*>(page);

      auto page_index = table.rootIndex_;

      if (node->nodeHeader_.nodeType_ == NodeType::kNodeLeaf) {
          auto cursor = find_key_in_leaf_node(node, page_index, key);
          // 若内存中已经有相应的数据，而且key还相同，就判定重复
          if (node->leafNodeBody_.cells[cursor.getCellIndex()].key_ == key) {
            std::cerr << "Duplicated key error!" << std::endl;
            exit(static_cast<int>(StatusCode::kDuplicatedKey));
          }

          return cursor;
      } else {
          std::cerr << "Unimplemented branch!" << std::endl;
          ::exit(static_cast<int>(StatusCode::kUnknownError));
      }
  }

 private:
  memory::Cursor find_key_in_leaf_node(LeafNode* node, u32 page_index, u32 key) {
    // 在std::array中找到合适的插入位置
    auto target_index = std::distance(std::begin(node->leafNodeBody_.cells),
                                     std::lower_bound(
                                         node->leafNodeBody_.cells.begin(),
                                         node->leafNodeBody_.cells.begin() + node->leafNodeHeader_.cellsCount_,
                                         key,
                                         [](const structure::Cell& cell, u32 key) { return cell.key_ < key; }
                                     )
    );

    return memory::Cursor(page_index, target_index);
  }
};

}   // namespace structure ends
