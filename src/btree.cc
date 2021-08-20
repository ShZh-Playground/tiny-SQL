#include "../include/btree.h"

using namespace structure;

#define page_index_to_leaf_node(x)  (reinterpret_cast<LeafNode*>(table.pager.GetPage(x)))

#define page_index_to_internal_node(x) (reinterpret_cast<InternalNode*>(table.pager.GetPage(x)))

NodeType structure::GetNodeType(memory::Table& table, usize page_index) {
  auto* raw_address = table.pager.GetPage(page_index);
  auto* header = reinterpret_cast<NodeHeader*>(raw_address);
  return header->node_type;
}

void structure::Indent(u32 level) {
  for (usize i = 0; i < level; ++i) {
    std::cout << "   ";
  }
}

static void PrintLeafNode(memory::Table& table, usize leaf_index, u32 indent_level) {
  auto* leaf_node = page_index_to_leaf_node(leaf_index);
  Indent(indent_level);
  std::cout << "- leaf(size " << leaf_node->leaf_node_header.cells_count << ")" << std::endl;
  for (usize index = 0; index < leaf_node->leaf_node_header.cells_count; ++index) {
    Indent(indent_level + 1);
    std::cout << "- " << leaf_node->leaf_node_body.cells[index].key_ << std::endl;
  }
}

static void PrintInternalNode(memory::Table& table, usize internal_index, u32 indent_level) {
  auto* internal_node = page_index_to_internal_node(internal_index);
  Indent(indent_level);
  std::cout << "- internal(size " << internal_node->internal_node_header.key_nums
            << ")" << std::endl;
  for (usize index = 0; index < internal_node->internal_node_header.key_nums;
       ++index) {
    PrintBtree(table,
        internal_node->internal_node_body.children[index].value_,
               indent_level + 1);
    Indent(indent_level + 1);
    std::cout << "- key " << internal_node->internal_node_body.children[index].key_
              << std::endl;
  }
  PrintBtree(table, internal_node->internal_node_body.rightest_child,
             indent_level + 1);
}

void structure::PrintBtree(memory::Table& table, usize page_index, u32 indent_level) {
  switch (GetNodeType(table, page_index)) {
    case NodeType::kNodeLeaf:
      PrintLeafNode(table, page_index, indent_level);
      break;
    case NodeType::kNodeInternal:
      PrintInternalNode(table, page_index, indent_level);
      break;
  }
}

void BTree::InsertLeafNode(memory::Table& table, memory::Cursor& cursor,
                                   u32 key, memory::Row row) {
  auto* node = page_index_to_leaf_node(cursor.page_index);

  if (node->leaf_node_header.cells_count >= kMaxCells) {
    SplitLeafNodeAndInsert(table, cursor, key, row);
    return;
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
}

memory::Cursor BTree::FindKey(memory::Table& table, u32 key) {
  // 查询的入口函数，从根结点开始查找
  switch (GetNodeType(table, table.root_index)) {
    case NodeType::kNodeLeaf:
      return FindKeyInLeafNode(table, table.root_index, key);
    case NodeType::kNodeInternal:
      return FindKeyInInternalNode(table, table.root_index, key);
  }
}

Cell* BTree::GetDestinationAddress(usize index, LeafNode* old_node,
                                     LeafNode* new_node) {
  auto* destination_node = index < kSplitLeafLeftCount ? old_node : new_node;
  u32 new_index = index % kSplitLeafLeftCount;
  auto* destination = &destination_node->leaf_node_body.cells[new_index];

  return destination;
}

memory::Cursor BTree::FindInInternalNode(memory::Table& table, usize page_index, u32 key) {
  auto* page = table.pager.GetPage(page_index);
  auto* node = reinterpret_cast<InternalNode*>(page);

  auto target_index = std::distance(
      std::begin(node->internal_node_body.children),
      std::lower_bound(node->internal_node_body.children.begin(),
                       node->internal_node_body.children.begin() +
                       node->internal_node_header.key_nums,
                       key, [](const structure::Child& child, u32 key) {
            return child.key_ < key;
          }));
  // 注意internal node不可能是end of table
  return memory::Cursor(&table, page_index, target_index, false);
}

memory::Cursor BTree::FindKeyInLeafNode(memory::Table& table, usize page_index, u32 key) {
  auto* node = page_index_to_leaf_node(page_index);

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

  return memory::Cursor(&table, page_index, target_index, is_at_end);
}

memory::Cursor BTree::FindKeyInInternalNode(memory::Table& table, usize page_index, u32 key) {
  // 先找到对应key所在的索引，注意还需要到children中查找真实的地址
  auto child_pos = FindInInternalNode(table, page_index, key);
  auto* node = page_index_to_internal_node(page_index);
  // 注意rightest带来的区别
  usize child_page_index = child_pos.cell_index >= node->internal_node_header.key_nums
                         ? node->internal_node_body.rightest_child
                         : node->internal_node_body.children[child_pos.cell_index].value_;

  switch (GetNodeType(table, child_page_index)) {
    // 递归查询internal node
    case NodeType::kNodeInternal:
      return FindKeyInInternalNode(table, child_page_index, key);
    // 已经到了叶子节点，那就直接查找
    case NodeType::kNodeLeaf:
      return FindKeyInLeafNode(table, child_page_index, key);
  }
}

void BTree::SplitLeafNodeAndInsert(memory::Table& table,
                                             memory::Cursor& cursor, u32 key,
                                             memory::Row row) {
  // 获取新旧两个node
  auto* old_node = page_index_to_leaf_node(cursor.page_index);
  usize new_page_index = table.pager.GetUnusedPage();
  auto* new_node = page_index_to_leaf_node(new_page_index);

  // 获取max key，以备后续过程使用
  u32 old_node_max_key = GetMaxKey(table, cursor.page_index);

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
  old_node->leaf_node_header.cells_count = kSplitLeafLeftCount;
  old_node->leaf_node_header.next_page_index = new_page_index;
  new_node->node_header.node_type = NodeType::kNodeLeaf;
  new_node->node_header.parent_pointer = old_node->node_header.parent_pointer;
  new_node->leaf_node_header.cells_count = kSplitLeafRightCount;

  // 如果该节点是根结点，那么就要特殊处理（分裂后引入新的根结点）
  if (cursor.page_index == table.root_index) {
    CreateNewRootLeaf(table, new_page_index);
    return;
  }
  // 如果不是，那就在父节点中更新这两个节点的信息
  // 1. 更新老节点的信息（即max key的信息）
  u32 old_node_new_max_key = GetMaxKey(table, cursor.page_index);
  UpdateInternalNodeKey(table, old_node->node_header.parent_pointer, old_node_max_key, old_node_new_max_key);
  // 2. 插入新节点信息
  InternalNodeInsert(table, old_node->node_header.parent_pointer, new_page_index);
}

// 把原来根结点的内容拷贝到一个新的左节点中
// 然后把这个根结点重新初始化为内部节点
void BTree::CreateNewRootLeaf(memory::Table& table,
                                  usize right_node_page_index) {
  // 拷贝到新的左节点中
  auto* root_node = page_index_to_leaf_node(table.root_index);
  auto left_node_page_index = table.pager.GetUnusedPage();
  auto* left_node = page_index_to_leaf_node(left_node_page_index);
  ::memcpy(left_node, root_node, sizeof(LeafNode));

  // 现在left node已经不是根结点，并且需要设置parent_pointer
  left_node->node_header.is_root = 0U;
  left_node->node_header.parent_pointer = table.root_index;
  left_node->leaf_node_header.next_page_index = right_node_page_index;
  // 给right node也设置一个parent pointer
  auto* right_node = page_index_to_leaf_node(right_node_page_index);
  right_node->node_header.parent_pointer = table.root_index;

  // 创建内部节点
  // 因为是新的节点所以只有一个成员
  auto* root_as_internal = reinterpret_cast<InternalNode*>(root_node);
  root_as_internal->node_header.node_type = NodeType::kNodeInternal;
  root_as_internal->internal_node_header.key_nums = 1;
  root_as_internal->internal_node_body.children[0].key_ = left_node->GetMaxKey();
  root_as_internal->internal_node_body.children[0].value_ = left_node_page_index;
  root_as_internal->internal_node_body.rightest_child = right_node_page_index;
}

u32 BTree::GetMaxKey(memory::Table& table, usize page_index) {
  switch (GetNodeType(table, page_index)) {
    case NodeType::kNodeInternal: {
      auto* internal_node = page_index_to_internal_node(page_index);
      usize rightest_page_index = internal_node->internal_node_body.rightest_child;
      return GetMaxKey(table, rightest_page_index);
    }
    case NodeType::kNodeLeaf: {
      auto* leaf_node = page_index_to_leaf_node(page_index);
      usize max_index = leaf_node->leaf_node_header.cells_count - 1;
      return leaf_node->leaf_node_body.cells[max_index].key_;
    }
  }
}

void BTree::UpdateInternalNodeKey(memory::Table& table, usize page_index,
                                  u32 old_key, u32 new_key) {
  // 注意rightest只不过是一个地址而已，更新key的时候可以不用管
  auto* node = page_index_to_internal_node(page_index);
  auto key_pos = FindInInternalNode(table, page_index, old_key);

  // 最右节点不用更新
  if (key_pos.cell_index != kMaxChild) {
    node->internal_node_body.children[key_pos.cell_index].key_ = new_key;
  }
}

void BTree::InternalNodeInsert(memory::Table& table, usize parent_index,
                                 usize child_index) {
  auto* parent = page_index_to_internal_node(parent_index);
  usize rightest_child_index = parent->internal_node_body.rightest_child;
  
  u32 child_max_key = GetMaxKey(table, child_index);
  u32 rightest_max_key = GetMaxKey(table, rightest_child_index);

  // 内部节点已经满了，进行分裂
  if (parent->internal_node_header.key_nums == kMaxChild) {
    SplitInternalNodeAndInsert(table, parent_index, child_max_key, child_index);
    return;
  }

  // 通过判断max key来决定是否成为最右节点
  if (child_max_key < rightest_max_key) {
    // 如果新节点还不至于成为最右的节点，那就直接插入到body的数组中
    usize insert_index = parent->internal_node_header.key_nums;
    parent->internal_node_body.children[insert_index].key_ = child_max_key;
    parent->internal_node_body.children[insert_index].value_ = child_index;
  } else {
    // 如果新节点是新的最右的节点，那就交换这两个成员的位置
    // 先把原来rightest的移到数组末端
    usize insert_index =  parent->internal_node_header.key_nums;
    parent->internal_node_body.children[insert_index].key_ = rightest_max_key;
    parent->internal_node_body.children[insert_index].value_ = rightest_child_index;
    // 再把新child作为rightest
    parent->internal_node_body.rightest_child = child_index;
  }
  // 更新老节点元信息
  // 新节点的parent pointer就不用更改了，之前已经改过了
  ++parent->internal_node_header.key_nums;
}

void BTree::SplitInternalNodeAndInsert(memory::Table& table, usize page_index,
                                       u32 key, u32 value) {
  // 获取新旧两个node
  auto* old_node = page_index_to_internal_node(page_index);
  usize new_node_page_index = table.pager.GetUnusedPage();
  auto* new_node = page_index_to_internal_node(new_node_page_index);
  usize old_rightest_child_index = old_node->internal_node_body.rightest_child;

  // 获取max key，以备后续过程使用
  u32 old_node_max_key = GetMaxKey(table, page_index);

  // 给原来的数据移动到它的位置上去
  // 1. 左边的节点赋值，只要改变rightest child即可
  old_node->internal_node_body.rightest_child = old_node->internal_node_body.children[kSplitInternalLeftCount].value_;
  // 2. 右边的节点赋值，不仅要改变数组，还要根据情况交换rightest child
  // P.S. 这里kSplitInternalLeftCount + 1中的加一都是算上了rightest child的情况
  for (usize index = kSplitInternalLeftCount + 1; index < old_node->internal_node_header.key_nums; ++index) {
    u32 new_index = index % (kSplitInternalLeftCount + 1);
    auto* destination = &new_node->internal_node_body.children[new_index];
    auto* source = &old_node->internal_node_body.children[index];

    ::memcpy(destination, source, sizeof(Child));
  }
  // 插入新数据，还是要注意rightest child
  u32 old_rightest_child = GetMaxKey(table, old_rightest_child_index);
  if (key < old_rightest_child) {
    new_node->internal_node_body.children[kSplitInternalRightCount - 1].key_ = key;
    new_node->internal_node_body.children[kSplitInternalRightCount - 1].value_ = value;
    new_node->internal_node_body.rightest_child = old_rightest_child_index;
  } else {
    new_node->internal_node_body.children[kSplitInternalRightCount - 1].key_ = old_rightest_child;
    new_node->internal_node_body.children[kSplitInternalRightCount - 1].value_ = old_rightest_child_index;
    new_node->internal_node_body.rightest_child = value;
  }

  // 给每个节点填充基本属性
  old_node->internal_node_header.key_nums = kSplitInternalLeftCount;
  new_node->node_header.node_type = NodeType::kNodeInternal;
  new_node->node_header.parent_pointer = old_node->node_header.parent_pointer;
  new_node->internal_node_header.key_nums = kSplitInternalRightCount;

  // 另外，如果该节点是根结点，就要特殊处理（分裂后引入新的根结点）
  if (page_index == table.root_index) {
    CreateNodeRootInternal(table, new_node_page_index);
    return;
  }
  // 如果不是，那就在父节点中更新这两个节点的信息
  // 1. 更新老节点的信息（即max key的信息）
  u32 old_node_new_max_key = GetMaxKey(table, page_index);
  UpdateInternalNodeKey(table, old_node->node_header.parent_pointer, old_node_max_key, old_node_new_max_key);
  // 2. 插入新节点信息
  InternalNodeInsert(table, old_node->node_header.parent_pointer, new_node_page_index);
}

void BTree::CreateNodeRootInternal(memory::Table& table,
                                   usize right_node_page_index) {
  // 拷贝到新的左节点中
  auto* root_node = page_index_to_internal_node(table.root_index);
  auto left_node_page_index = table.pager.GetUnusedPage();
  auto* left_node = page_index_to_internal_node(left_node_page_index);
  ::memcpy(left_node, root_node, sizeof(InternalNode));

  // 现在left node已经不是根结点，并且需要设置parent_pointer
  left_node->node_header.is_root = 0U;
  left_node->node_header.parent_pointer = table.root_index;
  // 给right node也设置一个parent pointer
  auto* right_node = page_index_to_internal_node(right_node_page_index);
  right_node->node_header.parent_pointer = table.root_index;

  // 创建新的内部节点
  // 因为是新的节点所以只有一个成员
  auto* root_as_internal = reinterpret_cast<InternalNode*>(root_node);
  root_as_internal->node_header.node_type = NodeType::kNodeInternal;
  root_as_internal->internal_node_header.key_nums = 1;
  root_as_internal->internal_node_body.children[0].key_ = GetMaxKey(table, left_node_page_index);
  root_as_internal->internal_node_body.children[0].value_ = left_node_page_index;
  root_as_internal->internal_node_body.rightest_child = right_node_page_index;
}
