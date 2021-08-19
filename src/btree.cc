#include "../include/btree.h"

using namespace structure;

NodeType structure::GetNodeType(Addr raw_address) {
    auto* header = reinterpret_cast<NodeHeader*>(raw_address);
    return header->node_type;
}

void structure::Indent(u32 level) {
  for (usize i = 0; i < level; ++i) {
    std::cout << " ";
  }
}

void structure::PrintBtree(memory::Table& table, usize page_index, u32 indent_level) {
  auto* page = table.pager.GetPage(page_index);
  auto node_type = GetNodeType(page);
  if (node_type == NodeType::kNodeLeaf) {
    auto* node = reinterpret_cast<LeafNode*>(page);
    Indent(indent_level);
    std::cout << "- leaf(size " << node->leaf_node_header.cells_count << ")" << std::endl;
    for (usize index = 0; index < node->leaf_node_header.cells_count; ++index) {
      Indent(indent_level + 1);
      std::cout << "- " << node->leaf_node_body.cells[index].key_ << std::endl;
    }
  } else if (node_type == NodeType::kNodeInternal) {
    auto* node = reinterpret_cast<InternalNode*>(page);
    Indent(indent_level);
    std::cout << "- internal(size " << node->internal_node_header.key_nums
              << ")" << std::endl;
    for (usize index = 0; index < node->internal_node_header.key_nums;
         ++index) {
      PrintBtree(table,
                  node->internal_node_body.children[index].value_.page_index,
                  indent_level + 1);
      Indent(indent_level + 1);
      std::cout << "- key " << node->internal_node_body.children[index].key_
                << std::endl;
    }
    PrintBtree(table, node->internal_node_body.rightest_child,
                indent_level + 1);
  }
}

StatusCode BTree::InsertLeafNode(memory::Table& table, memory::Cursor& cursor,
                                   u32 key, memory::Row row) {
  auto* page = table.pager.GetPage(cursor.page_index);
  auto* node = reinterpret_cast<LeafNode*>(page);

  if (node->leaf_node_header.cells_count >= kMaxCells) {
    return SplitLeafNodeAndInsert(table, cursor, key, row);
  }
  // Page里面的各个cell依然是一个数组。
  // 插入的时候把相应位置后面的元素都往后移一位
  u32 insert_index = cursor.cell_index;
  for (u32 index = kMaxCells - 1; index > insert_index; --index) {
    ::memcpy(&node->leaf_node_body.cells[index],
             &node->leaf_node_body.cells[index - 1], sizeof(Cell));
  }

  ++node->leaf_node_header.cells_count;
  node->leaf_node_body.cells[insert_index] = Pair(key, row);

  return StatusCode::kSuccess;
}
memory::Cursor BTree::FindKey(memory::Table& table, u32 key) {
  // 查找插入位置的时候，是从根结点开始查找
  auto page_index = table.root_index;
  auto* page = table.pager.GetPage(table.root_index);

  auto node_type = GetNodeType(page);
  if (node_type == NodeType::kNodeLeaf) {
    auto* node = reinterpret_cast<LeafNode*>(page);
    auto cursor = FindKeyInLeafNode(&table, node, page_index, key);

    return cursor;
  }
  if (node_type == NodeType::kNodeInternal) {
    // 找到internal node中对应的key
    auto* node = reinterpret_cast<InternalNode*>(page);
    auto cursor = FindKeyInInternalNode(&table, node, page_index, key);
    // 根据key-value对获取相应的地址
    auto child_page_index =
        node->internal_node_body.children[cursor.cell_index].value_.page_index;

    auto* page = table.pager.GetPage(child_page_index);
    auto node_type = GetNodeType(page);
    if (node_type == NodeType::kNodeLeaf) {
      auto* node = reinterpret_cast<LeafNode*>(page);
      return FindKeyInLeafNode(&table, node, child_page_index, key);
    } else if (node_type == NodeType::kNodeInternal) {
      auto* node = reinterpret_cast<InternalNode*>(page);
      return FindKeyInInternalNode(&table, node, child_page_index, key);
    }

    return cursor;
  }
}
Cell* BTree::GetDestinationAddress(usize index, LeafNode* old_node,
                                     LeafNode* new_node) {
  auto* destination_node = index < kSplitLeftCount ? old_node : new_node;
  u32 new_index = index % kSplitLeftCount;
  auto* destination = &destination_node->leaf_node_body.cells[new_index];

  return destination;
}
memory::Cursor BTree::FindKeyInLeafNode(memory::Table* table,
                                            LeafNode* node, usize page_index,
                                            u32 key) {
  // 在std::array中找到合适的插入位置
  auto target_index = std::distance(
      std::begin(node->leaf_node_body.cells),
      std::lower_bound(
          node->leaf_node_body.cells.begin(),
          node->leaf_node_body.cells.begin() + node->leaf_node_header.cells_count,
          key, [](const structure::Cell& cell, u32 key) {
            return cell.key_ < key;
          }));

  // 叶子节点，且节点总数=0，说明end of table了
  bool is_at_end = node->leaf_node_header.cells_count == 0;

  return memory::Cursor(table, page_index, target_index, is_at_end);
}
memory::Cursor BTree::FindKeyInInternalNode(memory::Table* table,
                                                InternalNode* node,
                                                usize page_index, u32 key) {
  auto target_index = std::distance(
      std::begin(node->internal_node_body.children),
      std::lower_bound(node->internal_node_body.children.begin(),
                       node->internal_node_body.children.begin() +
                           node->internal_node_header.key_nums,
                       key, [](const structure::Child& child, u32 key) {
                         return child.key_ < key;
                       }));
  // 注意internal node不可能是end of table
  return memory::Cursor(table, page_index, target_index, false);
}
StatusCode BTree::SplitLeafNodeAndInsert(memory::Table& table,
                                             memory::Cursor& cursor, u32 key,
                                             memory::Row row) {
  // 获取新旧两个node
  auto* old_node =
      reinterpret_cast<LeafNode*>(table.pager.GetPage(cursor.page_index));
  usize new_page_index = table.pager.GetUnusedPage();
  auto* new_node =
      reinterpret_cast<LeafNode*>(table.pager.GetPage(new_page_index));

  // Mac下command + ctrl + g修改同一个局部变量名
  for (usize index = 0; index <= old_node->leaf_node_header.cells_count;
       ++index) {
    // 确定每个数据的新地址
    auto* destination = GetDestinationAddress(index, old_node, new_node);

    // 注意给新数据单独留一个位置
    if (index < cursor.cell_index) {
      Cell* source = &old_node->leaf_node_body.cells[index];
      ::memcpy(destination, source, sizeof(Cell));
    } else if (index > cursor.cell_index) {
      Cell* source = &old_node->leaf_node_body.cells[index - 1];
      ::memcpy(destination, source, sizeof(Cell));
    }
  }
  // 插入新数据
  // 单独插是为了防止存在old node中覆盖原有的数据
  auto* insert_address =
      GetDestinationAddress(cursor.cell_index, old_node, new_node);
  insert_address->key_ = key;
  insert_address->value_ = row;

  // 给每个节点填充一下基本的属性
  old_node->leaf_node_header.cells_count = kSplitLeftCount;
  old_node->leaf_node_header.next_page_index = new_page_index;
  new_node->node_header.node_type = NodeType::kNodeLeaf;
  new_node->leaf_node_header.cells_count = kSplitRightCount;

  // 确定parentPointer
  if (cursor.page_index == table.root_index) {
    return CreateNewRoot(table, new_page_index);
  }

  std::cerr << "Unimplemented branch!" << std::endl;
  ::exit(-1);
}

StatusCode BTree::CreateNewRoot(memory::Table& table,
                                  usize right_node_page_index) {
  // 拷贝到新的左节点中
  auto* root_node =
      reinterpret_cast<LeafNode*>(table.pager.GetPage(table.root_index));
  auto left_node_page_index = table.pager.GetUnusedPage();
  auto* left_node =
      reinterpret_cast<LeafNode*>(table.pager.GetPage(left_node_page_index));
  ::memcpy(left_node, root_node, sizeof(LeafNode));
  left_node->node_header.is_root = 0U;    // Not root
  left_node->leaf_node_header.next_page_index = right_node_page_index;

  // 创建内部节点
  auto* root_as_internal = reinterpret_cast<InternalNode*>(root_node);
  root_as_internal->node_header.node_type = NodeType::kNodeInternal;
  root_as_internal->internal_node_header.key_nums = 1;
  root_as_internal->internal_node_body.children[0].key_ = left_node->get_max_key();
  root_as_internal->internal_node_body.children[0].value_.page_index =
      left_node_page_index;
  root_as_internal->internal_node_body.children[0].value_.next_page_index =
      right_node_page_index;
  root_as_internal->internal_node_body.rightest_child = right_node_page_index;

  return StatusCode::kSuccess;
}
