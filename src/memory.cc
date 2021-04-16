#include<cstring>

#include "../include/memory.h"

using memory::Pager;
using memory::Table;

// 内存中的数据
// 这里只能用全局变量实现
Table* table  = new Table("student.db");

// namespace中的非成员函数要加上namespace来标识
// 不然编译器会认为可能你又定义了一个函数
void memory::saveToMem(const Row& row, Byte* addr) {
  // 不采用默认的对齐方式，内存之间直接紧挨着
  ::memcpy(addr, &row.id, sizeof(row.id));
  ::memcpy(addr + sizeof(row.id), row.name, sizeof(row.name));
  ::memcpy(addr + sizeof(row.id) + sizeof(row.name), row.email,
         sizeof(row.email));
}

memory::Row memory::loadFromMem(Byte* addr) {
  memory::Row row{};
  ::memcpy(&(row.id), addr, sizeof(row.id));
  ::memcpy(row.name, addr + sizeof(row.id), sizeof(row.name));
  ::memcpy(row.email, addr + sizeof(row.id) + sizeof(row.name),
         sizeof(row.email));
  return row;
}

::uint32_t memory::getFileSize(std::fstream& file) {
  ::uint32_t size = file.seekg(0, std::ios::end).tellg();
  file.seekg(0, std::ios::beg);    // Rewind
  return size;
}

// fstream创造不存在的文件得用trunc, 想要读写还必须显示指定in和out
// 另外fstream不可copy，所以这里只能重复构造
Pager::Pager(const char* filename)
  : file_(filename, std::fstream::in | std::fstream::out | std::fstream::app) {
  this->fileSize_ = getFileSize(file_);
  this->totalPage_ = (this->fileSize_ + kPageSize - 1) / kPageSize;
  for (auto& page : this->pages_) {
    page = nullptr;
  }
}

void Pager::persist(::uint32_t remainedMem) {
  // 把完整的页：大小为kPageSize的保存下来
  for (int i = 0; i < this->totalPage_ - 1; ++i) {
    this->file_.write(this->pages_[i], kPageSize);
  }
  this->file_.write(this->pages_[this->totalPage_ - 1], remainedMem);
}

void Pager::close(::uint32_t remainedMem) {
  // 持久化到硬盘上
  persist(remainedMem);
  // 关闭文件流
  this->file_.close();
}

Pager::~Pager() {
  // 清空申请的堆内存
  for (auto& page : this->pages_) {
    if (page) {
      delete page;
      page = nullptr;
    }
  }
}

memory::Byte* Pager::getPage(::uint32_t index) {
  if (index >= kMaxPageNum) {
    std::cout << "Page index out of bound!" << std::endl;
    exit(-1);
  }
  // 把内存看成是磁盘的缓存
  // 没有在内存中找到，就从磁盘中找
  if (this->pages_[index] == nullptr) {
    this->pages_[index] = new Byte[kPageSize];

    // 这个页面在磁盘中
    // 如果这个条件不满足，说明添加了新的条目，磁盘中的文件也得更新了
    if (index < this->totalPage_) {
      this->file_.seekg(index * kPageSize, std::ios::beg);
      this->file_.read(this->pages_[index], kPageSize);
    } else {
      // 这个页面连磁盘都没有
      // 增加页数，之后写入磁盘
      ++this->totalPage_;
    }
  }

  return this->pages_[index];
}

Table::Table(const char* filename) {
  std::fstream file{ filename, std::fstream::in | std::fstream::out | std::fstream::app };
  // index初始值由持久化的文件决定
  this->index_ = getFileSize(file) / Row::getSize();
  this->pager_ = new Pager{ filename };
}

Table::~Table() {
  this->pager_->close(this->index_ * Row::getSize() % kPageSize);
  delete this->pager_;
}

memory::Byte* Table::getInsertAddr() {
  ::uint32_t pageIndex = this->index_ / this->kRowCountPerPage;
  ::uint16_t rowOffset = this->index_ % this->kRowCountPerPage;
  auto page = this->pager_->getPage(pageIndex);
  memory::Byte* insertAddr = page + rowOffset * memory::Row::getSize();

  return insertAddr;
}

void Table::insert(const Row& row) {
  auto* insertAddr = this->getInsertAddr();
  saveToMem(row, insertAddr);
  ++this->index_;
}

std::ostream& memory::operator<<(std::ostream& os, const memory::Table& table) {
  for (::uint32_t i = 0; i < table.index_; ++i) {
    ::uint32_t pageIndex = i / table.kRowCountPerPage;
    ::uint32_t rowOffset = i % table.kRowCountPerPage;
    Byte* addr = table.pager_->getPage(pageIndex) + rowOffset * Row::getSize();

    auto row = loadFromMem(addr);
    os << "( " << row.id << ", " << row.name << ", " << row.email << " )"
        << std::endl;
  }
  return os;
}
