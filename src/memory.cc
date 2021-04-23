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
    std::cerr << "Error: Wrong file size! Incomplete page apper!" << std::endl;
    exit(Error::kWrongFileSize);
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
    this->pages_[index] = reinterpret_cast<Addr>(new structure::LeafNode());
    
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
  auto* page = this->pager_.getPage(this->cursor_.getPageIndex());
  auto node = reinterpret_cast<structure::LeafNode*>(page);
  if (node->leafNodeHeader_.cellsCount_ == structure::kMaxCells) {
    std::cerr << "Page Full!!" << std::endl;
    exit(-1);
  }

  // 在std::array中找到合适的插入位置
  auto targetIndex = std::distance(std::begin(node->leafNodeBody_.cells), 
    std::lower_bound(
      node->leafNodeBody_.cells.begin(), 
      node->leafNodeBody_.cells.begin() + node->leafNodeHeader_.cellsCount_,
      cell.key_, 
      [](const structure::Cell& cell, u32 key) { return cell.key_ < key; }
    )
  );
  // 不允许出现相同的key
  if (node->leafNodeBody_.cells[targetIndex].key_ == cell.key_) {
    std::cerr << "Duplicated key error!" << std::endl;
    exit(kDuplicatedKey);
  }

  // 将目标元素后面的元素向后移一个cell的大小
  if (targetIndex < node->leafNodeHeader_.cellsCount_) {
    // 获取移动的地址
    Addr srcAddr = reinterpret_cast<Addr>(&node->leafNodeBody_) + \
      targetIndex * sizeof(cell);           // 移动的源地址
    Addr dstAddr = srcAddr + sizeof(cell);  // 移动的目标地址
    // 获取移动的大小
    auto numToMove = node->leafNodeHeader_.cellsCount_ - targetIndex + 1;
    auto sizeToMove = numToMove * sizeof(cell);
    // 利用memmove处理重叠的移动
    ::memmove(dstAddr, srcAddr, sizeToMove);
  }

  // 插入到目标位置
  Addr insertAddr = reinterpret_cast<Addr>(&node->leafNodeBody_) + \
      targetIndex * sizeof(cell);
  saveToMemory(insertAddr, cell);
  ++node->leafNodeHeader_.cellsCount_;
  ++this->cursor_;
}

std::ostream& memory::operator<<(std::ostream& os, memory::Table& table) {
  auto* page = table.pager_.getPage(table.cursor_.getPageIndex());
  auto node = reinterpret_cast<structure::LeafNode*>(page);

  for (u32 i = 0; i < node->leafNodeHeader_.cellsCount_; ++i) {
    std::cout << node->leafNodeBody_.cells[i].value_ << std::endl;
  }

  return os;
}

std::ostream& memory::operator<<(std::ostream& os, const memory::Row& row) {
  os << "( " << row.id << ", " << row.name << ", " << row.email << " )";
  return os;
}
