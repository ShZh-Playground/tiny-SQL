#include<cstring>
#include<fstream>
#include<filesystem>

#include "../include/cursor.h"
#include "../include/memory.h"

using memory::Pager;
using memory::Table;

// 内存中的数据
// 这里只能用全局变量实现
const char* filename = "student.db";
auto file = std::filesystem::exists(filename) ?
  std::fstream { filename, std::ios::in | std::ios::out } :
  std::fstream { filename, std::ios::in | std::ios::out | std::ios::app};
Table* table  = new Table(file);

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
  memory::Row row;
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
Pager::Pager(std::fstream& file) : file_(file) {
  this->fileSize_ = getFileSize(file_);
  this->totalPage_ = (this->fileSize_ + kPageSize - 1) / kPageSize;
  for (auto& page : this->pages_) {
    page = nullptr;
  }
}

void Pager::persist(::uint32_t remainedMem) {
  this->file_.seekp(0, std::ios::beg);
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
  if (this->file_.is_open()) {
    this->file_.close();
  }
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
  // read没有读满会使failbit置位
  // 后面对文件的操作统统无效
  this->file_.clear();
  return this->pages_[index];
}

Table::Table(std::fstream& file) {
  // index初始值由持久化的文件决定
  this->rowNum_ = getFileSize(file) / Row::getSize();
  this->cursor_ = new cursor::Cursor{ *this, this->rowNum_ - 1 };
  this->pager_ = new Pager{ file };
}

Table::~Table() {
  // Pass remainde byte
  // 总数为索引 + 1
  this->cursor_->setEnd();
  this->pager_->close(this->cursor_->getCurIndex() * Row::getSize() % kPageSize);
  delete this->pager_;
  delete this->cursor_;
}

memory::Byte* Table::getInsertAddr() {
  auto pageIndex = this->cursor_->getCurIndex() / this->kRowCountPerPage;
  auto rowOffset = this->cursor_->getCurIndex() % this->kRowCountPerPage;
  auto* page = this->pager_->getPage(pageIndex);
  memory::Byte* insertAddr = page + rowOffset * memory::Row::getSize();

  return insertAddr;
}

void Table::insert(const Row& row) {
  this->cursor_->setEnd();
  ++this->rowNum_;
  auto* insertAddr = this->getInsertAddr();
  saveToMem(row, insertAddr);
}

std::ostream& memory::operator<<(std::ostream& os, const memory::Table& table) {
  table.cursor_->setBegin();

  while (!table.cursor_->isEnd()) {
    auto index = table.cursor_->getCurIndex();
    auto pageIndex = index / table.kRowCountPerPage;
    auto rowOffset = index % table.kRowCountPerPage;
    auto addr = table.pager_->getPage(pageIndex) + rowOffset * Row::getSize();

    auto row = loadFromMem(addr);
    os << "( " << row.id << ", " << row.name << ", " << row.email << " )"
        << std::endl;
    
    table.cursor_->advance();
  }

  return os;
}
