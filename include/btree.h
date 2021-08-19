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
  u32 page_index;
  u32 next_page_index;
};

enum class NodeType : u8 {
  kNodeLeaf,
  kNodeInternal,
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
  u32 key_nums_;
};

using Cell = Pair<u32, memory::Row>;

// key和page_num，next_page_num对
using Child = Pair<u32, PageInfo>;

constexpr u32 kMaxCells =  \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(LeafNodeHeader)) / sizeof(Cell);

constexpr u32 kSplitRightCount = (1 + kMaxCells) / 2;

constexpr u32 kSplitLeftCount = (1 + kMaxCells) - kSplitRightCount;

constexpr u32 kMaxChild = \
  (memory::kPageSize - sizeof(NodeHeader) - sizeof(InternalNodeHeader)) / sizeof(Child);

struct LeafNodeBody {
  std::array<Cell, kMaxCells> cells;
};

struct InternalNodeBody {
  std::array<Child, kMaxChild> childs;
  u32 rightestChild_;
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

  u32 get_max_key() {
    u32 max_index = this->leafNodeHeader_.cellsCount_ - 1;
    u32 max_key = this->leafNodeBody_.cells[max_index].key_;
    return max_key;
  }
};

struct alignas(memory::kPageSize) InternalNode {
  NodeHeader          nodeHeader_;
  InternalNodeHeader  internalNodeHeader_;
  InternalNodeBody    internalNodeBody_;
};

#pragma pack(pop)

NodeType get_node_type(Addr raw_address);

class BTree {
 public:
  StatusCode insert_leaf_node(memory::Table& table, memory::Cursor& cursor, u32 key, memory::Row row) {
    auto* page = table.pager_.getPage(cursor.getPageIndex());
    auto* node = reinterpret_cast<LeafNode*>(page);

    if (node->leafNodeHeader_.cellsCount_ >= kMaxCells) {
      return split_leaf_node_and_insert(table, cursor, key, row);
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
      // 查找插入位置的时候，是从根结点开始查找
      auto page_index = table.rootIndex_;
      auto* page = table.pager_.getPage(table.rootIndex_);

      auto node_type = get_node_type(page);
      if (node_type == NodeType::kNodeLeaf) {
          auto* node = reinterpret_cast<LeafNode*>(page);
          auto cursor = find_key_in_leaf_node(node, page_index, key);

          return cursor;
      } else if (node_type == NodeType::kNodeInternal) {
        // 找到internal node中对应的key
        auto* node = reinterpret_cast<InternalNode*>(page);
        auto cursor = find_key_in_internal_node(node, page_index, key);
        // 根据key-value对获取相应的地址
        auto child_page_index = node->internalNodeBody_.childs[cursor.getCellIndex()].value_.page_index;

        auto* page = table.pager_.getPage(child_page_index);
        auto node_type = get_node_type(page);
        if (node_type == NodeType::kNodeLeaf) {
          auto* node = reinterpret_cast<LeafNode*>(page);
          return find_key_in_leaf_node(node, child_page_index, key);
        } else if (node_type == NodeType::kNodeInternal) {
          auto* node = reinterpret_cast<InternalNode*>(page);
          return find_key_in_internal_node(node, child_page_index, key);
        }

        return cursor;
      }
  }

 private:
  Cell* get_destination_address(u32 index, LeafNode* old_node, LeafNode* new_node) {
    auto* destination_node = index < kSplitLeftCount? old_node : new_node;
    u32 new_index = index % kSplitLeftCount;
    auto* destination = &destination_node->leafNodeBody_.cells[new_index];

    return destination;
  }

  memory::Cursor find_key_in_leaf_node(LeafNode* node, u32 page_index, u32 key) {
    // 在std::array中找到合适的插入位置
    auto target_index = std::distance(
        std::begin(node->leafNodeBody_.cells),
        std::lower_bound(
            node->leafNodeBody_.cells.begin(),
            node->leafNodeBody_.cells.begin() + node->leafNodeHeader_.cellsCount_,
            key,
            [](const structure::Cell& cell, u32 key) { return cell.key_ < key; }
        )
    );

    // 若内存中已经有相应的数据，而且key还相同，就判定重复
    if (node->leafNodeBody_.cells[target_index].key_ == key) {
      std::cerr << "Duplicated key error!" << std::endl;
      exit(static_cast<int>(StatusCode::kDuplicatedKey));
    }

    return memory::Cursor(page_index, target_index);
  }

  memory::Cursor find_key_in_internal_node(InternalNode* node, u32 page_index, u32 key) {
    auto target_index = std::distance(
        std::begin(node->internalNodeBody_.childs),
          std::lower_bound(
            node->internalNodeBody_.childs.begin(),
            node->internalNodeBody_.childs.begin() + node->internalNodeHeader_.key_nums_,
            key,
            [](const structure::Child& child, u32 key) { return child.key_ < key; }
        )
    );

    return memory::Cursor(page_index, target_index);
  }

  StatusCode split_leaf_node_and_insert(memory::Table& table, memory::Cursor& cursor, u32 key, memory::Row row) {
    // 获取新旧两个node
    auto* old_node = reinterpret_cast<LeafNode*>(table.pager_.getPage(cursor.getPageIndex()));
    u32 new_page_index = table.pager_.get_unused_page();
    auto* new_node = reinterpret_cast<LeafNode*>(table.pager_.getPage(new_page_index));

    // Mac下command + ctrl + g修改同一个局部变量名
    for (u32 index = 0; index <= old_node->leafNodeHeader_.cellsCount_; ++index) {
      // 确定每个数据的新地址
      auto* destination = get_destination_address(index, old_node, new_node);

      // 注意给新数据单独留一个位置
      if (index < cursor.getCellIndex()) {
        Cell* source = &old_node->leafNodeBody_.cells[index];
        ::memcpy(destination, source, sizeof(Cell));
      } else if (index > cursor.getCellIndex()) {
        Cell* source = &old_node->leafNodeBody_.cells[index - 1];
        ::memcpy(destination, source, sizeof(Cell));
      }
    }
    // 插入新数据
    // 单独插是为了防止存在old node中覆盖原有的数据
    auto* insert_address = get_destination_address(cursor.getCellIndex(), old_node, new_node);
    insert_address->key_ = key;
    insert_address->value_ = row;

    // 给每个节点填充一下基本的属性
    old_node->leafNodeHeader_.cellsCount_ = kSplitLeftCount;
    new_node->nodeHeader_.nodeType_ = NodeType::kNodeLeaf;
    new_node->leafNodeHeader_.cellsCount_ = kSplitRightCount;

    // 确定parentPointer
    if (cursor.getPageIndex() == 0) {
      return create_new_root(table, new_page_index);
    } else {
      std::cerr << "Unimplemented branch!" << std::endl;
      ::exit(-1);
    }

    return StatusCode::kSuccess;
  }

  // 把原来根结点的内容拷贝到一个新的左节点中
  // 然后把这个根结点重新初始化为内部节点
  StatusCode create_new_root(memory::Table& table, u32 right_node_page_index) {
    // 拷贝到新的左节点中
    auto* root_node = reinterpret_cast<LeafNode*>(table.pager_.getPage(table.rootIndex_));
    auto left_node_page_index = table.pager_.get_unused_page();
    auto* left_node = reinterpret_cast<LeafNode*>(table.pager_.getPage(left_node_page_index));
    ::memcpy(left_node, root_node, sizeof(LeafNode));
    left_node->nodeHeader_.isRoot_ = false;

    // 创建内部节点
    auto* root_as_internal = reinterpret_cast<InternalNode*>(root_node);
    root_as_internal->nodeHeader_.nodeType_ = NodeType::kNodeInternal;
    root_as_internal->internalNodeHeader_.key_nums_ = 1;
    root_as_internal->internalNodeBody_.childs[0].key_ = left_node->get_max_key();
    root_as_internal->internalNodeBody_.childs[0].value_.page_index = left_node_page_index;
    root_as_internal->internalNodeBody_.childs[0].value_.next_page_index = right_node_page_index;
    root_as_internal->internalNodeBody_.rightestChild_ = right_node_page_index;

    return StatusCode::kSuccess;
  }
};

void indent(u32 level);

void print_btree(memory::Table& table, u32 page_index, u32 indent_level);

}   // namespace structure ends
