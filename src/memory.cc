#include "../include/def.h"
#include "../include/btree.h"

#include <algorithm>
#include <filesystem>
#include <memory.h>


using memory::Cursor;
using memory::Pager;
using memory::Table;

// 内存中的数据
// 这里只能用全局变量实现
const char* filename = "student.db";
auto file =
    std::filesystem::exists(filename)
        ? std::fstream{filename, std::ios::in | std::ios::out}
        : std::fstream{filename, std::ios::in | std::ios::out | std::ios::app};
Table* table = new Table(file);
structure::BTree btree = structure::BTree();

static u32 memory::GetFileSize(std::fstream& file) {
  u32 size = file.seekg(0, std::ios::end).tellg();
  file.seekg(0, std::ios::beg);  // Rewind
  return size;
}

// 在leaf node上advance
void Cursor::Advance() {
  auto* page = this->table->pager.GetPage(this->page_index);
  auto* node = reinterpret_cast<structure::LeafNode*>(page);

  if (this->cell_index < node->leaf_node_header.cells_count) {
    ++this->cell_index;
  }

  if (this->cell_index == node->leaf_node_header.cells_count) {
    if (node->leaf_node_header.next_page_index == 0) {
      this->end_of_table = true;
    } else {
      this->page_index = node->leaf_node_header.next_page_index;
      this->cell_index = 0;
    }
  }
}

Cursor Cursor::operator++() {
  this->Advance();
  return *this;
}

Cursor Cursor::operator++([[maybe_unused]] int n) {
  auto ret = *this;
  this->Advance();
  return ret;
}

Pager::Pager(std::fstream& file) : file(file) {
  this->file_size = GetFileSize(file);
  if (this->file_size % kPageSize != 0) {
    std::cerr << "Fatal Error: Wrong file size! Incomplete page apper!" << std::endl;
    ::exit(static_cast<int>(StatusCode::kWrongFileSize));
  }

  this->total_page = (this->file_size + kPageSize - 1) / kPageSize;
  for (auto& page : this->pages) {
    page = nullptr;
  }
}

void Pager::Persist() {
  this->file.seekp(0, std::ios::beg);
  for (int i = 0; i < this->total_page; ++i) {
    this->file.write(this->pages[i], kPageSize);
  }
}

Pager::~Pager() noexcept {
  // 持久化到硬盘上
  Persist();
  // 关闭文件流
  if (this->file.is_open()) {
    this->file.close();
  }
  // 清空申请的堆内存
  for (auto& page : this->pages) {
    if (page) {
      delete reinterpret_cast<structure::LeafNode*>(page);
      page = nullptr;
    }
  }
}

Addr Pager::GetPage(u32 index) {
  if (index >= kMaxPageNum) {
    std::cerr << "Error: page index out of bound!" << std::endl;
    exit(-1);
  }
  // 把内存看成是磁盘的缓存
  // 没有在内存中找到，就从磁盘中找
  if (this->pages[index] == nullptr) {
    this->pages[index] = reinterpret_cast<Addr>(new structure::LeafNode(static_cast<u8>(NULL)));

    // 这个页面在磁盘中
    // 如果这个条件不满足，说明添加了新的条目，磁盘中的文件也得更新了
    if (index < this->total_page) {
      this->file.seekg(index * kPageSize, std::ios::beg);
      this->file.read(this->pages[index], kPageSize);

      // read没有读满会使failbit置位
      // 后面对文件的操作统统无效
      // 但是我们现在操作的基本单元就是cell了
      this->file.clear();
    } else {
      // 这个页面连磁盘都没有
      // 增加页数，之后写入磁盘
      ++this->total_page;
    }
  }

  return this->pages[index];
}

u32 memory::Pager::GetUnusedPage() {
  // 找到一个未分配的page，分配内存并返回page_num
  // 如果没有找到未分配的（即，所有page均已分配），则返回full page error
  for (int page_index = 0; page_index < kMaxPageNum; ++page_index) {
    auto& page = this->pages[page_index];
    if (!page) {
      ++this->total_page;
      page = reinterpret_cast<Addr>(new structure::LeafNode(static_cast<u8>(NULL)));
      return page_index;
    }
  }
  std::cerr << "Page full error!" << std::endl;
  ::exit(static_cast<int>(StatusCode::kPageFullError));
}

Table::Table(std::fstream& file):
  pager(Pager{file}),
  root_index(0) {}

template<>
void Table::insert(const structure::Cell& cell) {
  auto cursor = structure::BTree::FindKey(*this, cell.key_);

  usize page_index = cursor.page_index;
  auto* page = this->pager.GetPage(page_index);
  auto* node = reinterpret_cast<structure::LeafNode*>(page);
  usize cell_index = cursor.cell_index;
  auto target_insert_pos_key = node->leaf_node_body.cells[cell_index].key_;
  if (target_insert_pos_key == cell.key_) {
    std::cerr << "Duplicated key error!" << std::endl;
    exit(static_cast<int>(StatusCode::kDuplicatedKey));
  }

  structure::BTree::InsertLeafNode(*this, cursor, cell.key_, cell.value_);
}

std::ostream& memory::operator<<(std::ostream& os, memory::Table& table) {
  auto leaf_node_cursor = table.GetStart();

  while (!leaf_node_cursor.end_of_table) {
    usize page_index = leaf_node_cursor.page_index;
    auto* page = table.pager.GetPage(page_index);
    auto* leaf_node = reinterpret_cast<structure::LeafNode*>(page);

    usize cell_index = leaf_node_cursor.cell_index;
    std::cout << leaf_node->leaf_node_body.cells[cell_index].value_ << std::endl;

    ++leaf_node_cursor;
  }

  return os;
}
Cursor memory::Table::GetStart() {
  // 找到最左边的叶子节点，返回对应的cursor
  return structure::BTree::FindKey(*this, 0);
}

std::ostream& memory::operator<<(std::ostream& os, const memory::Row& row) {
  os << "( " << row.id << ", " << row.name << ", " << row.email << " )";
  return os;
}
