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

struct PageInfo {
  usize page_index;
  usize next_page_index;
};

enum class NodeType : u8 {
  kNodeLeaf,
  kNodeInternal,
};

struct NodeHeader {
  NodeType node_type;
  u8 is_root;
  usize parent_pointer;
};

struct LeafNodeHeader {
  usize cells_count;
  usize next_page_index;
};

struct InternalNodeHeader {
  usize key_nums;
};

using Cell = Pair<u32, memory::Row>;

// key和page_num，next_page_num对
using Child = Pair<u32, PageInfo>;

constexpr usize kMaxCells =  \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(LeafNodeHeader)) / sizeof(Cell);

constexpr usize kSplitRightCount = (1 + kMaxCells) / 2;

constexpr usize kSplitLeftCount = (1 + kMaxCells) - kSplitRightCount;

constexpr usize kMaxChild = \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(InternalNodeHeader)) / sizeof(Child);

struct LeafNodeBody {
  std::array<Cell, kMaxCells> cells;
};

struct InternalNodeBody {
  std::array<Child, kMaxChild> children;
  usize rightest_child;
};

struct alignas(memory::kPageSize) LeafNode {
  NodeHeader node_header;
  LeafNodeHeader leaf_node_header;
  LeafNodeBody leaf_node_body;

  LeafNode(usize parent_pointer) {
    this->node_header = NodeHeader{
        .node_type = NodeType::kNodeLeaf,
        .is_root = false,
        .parent_pointer = parent_pointer,
    };
    this->leaf_node_header = LeafNodeHeader{
        .cells_count = 0,
        .next_page_index = 0,  // 0代表没有next page
    };
    this->leaf_node_body = LeafNodeBody{.cells = std::array<Cell, kMaxCells>()};
  }

  usize get_max_key() {
    usize max_index = this->leaf_node_header.cells_count - 1;
    usize max_key = this->leaf_node_body.cells[max_index].key_;
    return max_key;
  }
};

struct alignas(memory::kPageSize) InternalNode {
  NodeHeader node_header;
  InternalNodeHeader internal_node_header;
  InternalNodeBody internal_node_body;
};

#pragma pack(pop)

NodeType GetNodeType(Addr raw_address);

class BTree {
 public:
  static StatusCode InsertLeafNode(memory::Table& table, memory::Cursor& cursor,
                            u32 key, memory::Row row);

  static memory::Cursor FindKey(memory::Table& table, u32 key);

 private:
  static Cell* GetDestinationAddress(usize index, LeafNode* old_node,
                                     LeafNode* new_node);

  static memory::Cursor FindKeyInLeafNode(memory::Table* table, LeafNode* node,
                                          usize page_index, u32 key);

  static memory::Cursor FindKeyInInternalNode(memory::Table* table,
                                              InternalNode* node,
                                              usize page_index, u32 key);

  static StatusCode SplitLeafNodeAndInsert(memory::Table& table,
                                           memory::Cursor& cursor, u32 key,
                                           memory::Row row);

  // 把原来根结点的内容拷贝到一个新的左节点中
  // 然后把这个根结点重新初始化为内部节点
  static StatusCode CreateNewRoot(memory::Table& table,
                                  usize right_node_page_index);
};

void Indent(u32 level);

void PrintBtree(memory::Table& table, usize page_index, u32 indent_level);

}   // namespace structure ends
