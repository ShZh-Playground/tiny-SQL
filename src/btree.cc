#include "../include/btree.h"

using namespace structure;

void structure::indent(u32 level) {
  for (usize i = 0; i < level; ++i) {
    std::cout << " ";
  }
}

void structure::print_btree(memory::Table& table, Addr address, u32 indent_level) {
  auto* header = reinterpret_cast<NodeHeader*>(address);

  if (header->nodeType_ == NodeType::kNodeLeaf) {
    auto* node = reinterpret_cast<LeafNode*>(address);
    indent(indent_level);
    std::cout << "- leaf(size " << node->leafNodeHeader_.cellsCount_ << ")" << std::endl;
    for (usize index = 0; index < node->leafNodeHeader_.cellsCount_; ++index) {
      indent(indent_level + 1);
      std::cout << "- " << node->leafNodeBody_.cells[index].key_ << std::endl;
    }
  } else if (header->nodeType_ == NodeType::kNodeInternal) {
    auto* node = reinterpret_cast<InternalNode*>(address);
    indent(indent_level);
    std::cout << "- internal(size " << node->internalNodeHeader_.key_nums_ << ")" << std::endl;
    for (usize index = 0; index < node->internalNodeHeader_.key_nums_; ++index) {
      print_btree(table, reinterpret_cast<Addr>(node->internalNodeBody_.childs[index].value_), indent_level + 1);
      indent(indent_level + 1);
      std::cout << "- key " << node->internalNodeBody_.childs[index].key_ << std::endl;
    }
    print_btree(table, reinterpret_cast<Addr>(node->internalNodeBody_.rightestChild_), indent_level + 1);
  }
}
