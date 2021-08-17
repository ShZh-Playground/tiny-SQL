#include "../include/def.h"
#include "../include/btree.h"
#include "../include/memory.h"

#include <cstring>
#include <fstream>
#include <algorithm>
#include <filesystem>

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

static u32 memory::getFileSize(std::fstream& file) {
  u32 size = file.seekg(0, std::ios::end).tellg();
  file.seekg(0, std::ios::beg);  // Rewind
  return size;
}

void Cursor::advance() {
  if (this->cellIndex_ < structure::kMaxCells) {
    ++this->cellIndex_;
  } else {
    std::cerr << "Full page!" << std::endl;
    exit(-1);
  }
}

Cursor Cursor::operator++() {
  this->advance();
  return *this;
}

Cursor Cursor::operator++([[maybe_unused]] int n) {
  auto ret = *this;
  this->advance();
  return ret;
}

Pager::Pager(std::fstream& file) : file_(file) {
  this->fileSize_ = getFileSize(file_);
  if (this->fileSize_ % kPageSize != 0) {
    std::cerr << "Fatal Error: Wrong file size! Incomplete page apper!" << std::endl;
    ::exit(static_cast<int>(StatusCode::kWrongFileSize));
  }

  this->totalPage_ = (this->fileSize_ + kPageSize - 1) / kPageSize;
  for (auto& page : this->pages_) {
    page = nullptr;
  }
}

void Pager::persist() {
  this->file_.seekp(0, std::ios::beg);
  for (int i = 0; i < this->totalPage_; ++i) {
    this->file_.write(this->pages_[i], kPageSize);
  }
}

Pager::~Pager() noexcept {
  // 持久化到硬盘上
  persist();
  // 关闭文件流
  if (this->file_.is_open()) {
    this->file_.close();
  }
  // 清空申请的堆内存
  for (auto& page : this->pages_) {
    if (page) {
      delete reinterpret_cast<structure::LeafNode*>(page);
      page = nullptr;
    }
  }
}

Addr Pager::getPage(u32 index) {
  if (index >= kMaxPageNum) {
    std::cerr << "Error: page index out of bound!" << std::endl;
    exit(-1);
  }
  // 把内存看成是磁盘的缓存
  // 没有在内存中找到，就从磁盘中找
  if (this->pages_[index] == nullptr) {
    this->pages_[index] = reinterpret_cast<Addr>(new structure::LeafNode((u8)NULL));
    
    // 这个页面在磁盘中
    // 如果这个条件不满足，说明添加了新的条目，磁盘中的文件也得更新了
    if (index < this->totalPage_) {
      this->file_.seekg(index * kPageSize, std::ios::beg);
      this->file_.read(this->pages_[index], kPageSize);

      // read没有读满会使failbit置位
      // 后面对文件的操作统统无效
      // 但是我们现在操作的基本单元就是cell了
      this->file_.clear();
    } else {
      // 这个页面连磁盘都没有
      // 增加页数，之后写入磁盘
      ++this->totalPage_;
    }
  }

  return this->pages_[index];
}

Table::Table(std::fstream& file): 
  pager_(Pager{file}), 
  cursor_(Cursor{}), 
  rootIndex_(0) {}

template<>
void Table::insert(const structure::Cell& cell) {
  auto cursor = btree.find_key(*this, cell.key_);

  btree.insert_leaf_node(*this, cursor, cell.key_, cell.value_);

  ++this->cursor_;
}

std::ostream& memory::operator<<(std::ostream& os, memory::Table& table) {
  auto* page = table.pager_.getPage(table.cursor_.getPageIndex());
  auto* node = reinterpret_cast<structure::LeafNode*>(page);

  for (u32 i = 0; i < node->leafNodeHeader_.cellsCount_; ++i) {
    std::cout << node->leafNodeBody_.cells[i].value_ << std::endl;
  }

  return os;
}

std::ostream& memory::operator<<(std::ostream& os, const memory::Row& row) {
  os << "( " << row.id << ", " << row.name << ", " << row.email << " )";
  return os;
}
