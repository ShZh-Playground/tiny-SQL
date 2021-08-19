#include "../include/btree.h"

using namespace structure;

NodeType structure::get_node_type(Addr raw_address) {
    auto* header = reinterpret_cast<NodeHeader*>(raw_address);
    return header->nodeType_;
}

void structure::indent(u32 level) {
  for (usize i = 0; i < level; ++i) {
    std::cout << " ";
  }
}

void structure::print_btree(memory::Table& table, u32 page_index, u32 indent_level) {
  auto* page = table.pager_.getPage(page_index);
  auto node_type = get_node_type(page);
  if (node_type == NodeType::kNodeLeaf) {
    auto* node = reinterpret_cast<LeafNode*>(page);
    indent(indent_level);
    std::cout << "- leaf(size " << node->leafNodeHeader_.cellsCount_ << ")" << std::endl;
    for (usize index = 0; index < node->leafNodeHeader_.cellsCount_; ++index) {
      indent(indent_level + 1);
      std::cout << "- " << node->leafNodeBody_.cells[index].key_ << std::endl;
    }
  } else if (node_type == NodeType::kNodeInternal) {
    auto* node = reinterpret_cast<InternalNode*>(page);
    indent(indent_level);
    std::cout << "- internal(size " << node->internalNodeHeader_.key_nums_ << ")" << std::endl;
    for (usize index = 0; index < node->internalNodeHeader_.key_nums_; ++index) {
      print_btree(table, node->internalNodeBody_.childs[index].value_.page_index, indent_level + 1);
      indent(indent_level + 1);
      std::cout << "- key " << node->internalNodeBody_.childs[index].key_ << std::endl;
    }
    print_btree(table, node->internalNodeBody_.rightestChild_, indent_level + 1);
  }
}
