#ifndef TABLE_H__
#define TABLE_H__

#include<iostream>
#include<cstdint>
#include<memory>

namespace memory {

using Byte = char;

constexpr int kNameMaxLength = 20;
constexpr int kEmailMaxLength = 50;

struct Row {
  std::uint32_t id;
  char name[kNameMaxLength];
  char email[kEmailMaxLength];

  constexpr static std::uint32_t getSize() {
    return sizeof(id) + sizeof(name) + sizeof(email);
  }
};

void saveToMem(const Row& row, Byte* addr);

Row loadFromMem(Byte* addr);

constexpr std::uint32_t kPageSize = 4096;  // 和大多数操作系统分页的大小一样，4096B
constexpr std::uint32_t kMaxPageNum = 50;
// Table由各种页面组成
class Table {
 private:
  std::uint32_t index_;    // Table需要自己维护一个递增的索引
  Byte* pages_[kMaxPageNum];
  // 内存管理——如何把一个对象放在内存中
  // 答案是获取一个页面最多能放几个对象
  const std::uint32_t rowCountPerPage = kPageSize / Row::getSize();

  Byte* getInsertAddr();

 public:
  Table(): index_(0) {
    for (auto& page : this->pages_) {
      page = nullptr;
    }
  }
  ~Table() {
    // 将分配的内存作为条件，可以在没有分配过的page上终止
    for (int i = 0; this->pages_[i]; ++i) {
      delete this->pages_[i];
    }
  }

  void insert(const Row& row);

  friend std::ostream& operator<<(std::ostream& os, const Table& table) {
    for (int i = 0; i < table.index_; ++i) {
      std::uint32_t pageIndex = i / table.rowCountPerPage;
      std::uint32_t rowOffset = i % table.rowCountPerPage;
      Byte* addr = table.pages_[pageIndex] + rowOffset * Row::getSize();

      auto row = loadFromMem(addr);
      os << "( " <<row.id << ", " << row.name << ", " << row.email << " )" << std::endl;
    }
    return os;
  }
};

}  // Namespace memory ends

#endif // TABLE_H__ ends