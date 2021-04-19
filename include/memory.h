#ifndef TABLE_H__
#define TABLE_H__

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

#include "cursor.h"

namespace cursor {
class Cursor;
}

namespace memory {

using Byte = char;

struct Row;

// 和大多数操作系统分页的大小一样，4096B
constexpr ::uint32_t kPageSize = 4096;
constexpr ::uint32_t kMaxPageNum = 50;
constexpr ::uint32_t kNameMaxLength = 20;
constexpr ::uint32_t kEmailMaxLength = 50;

Row loadFromMem(Byte* addr);

void saveToMem(const Row& row, Byte* addr);

::uint32_t getFileSize(std::fstream& file);

struct Row {
  ::uint32_t id;

  char name[kNameMaxLength];

  char email[kEmailMaxLength];

  constexpr static ::uint32_t getSize() {
    return sizeof(id) + sizeof(name) + sizeof(email);
  }
};

class Pager {
 private:
  Byte* pages_[kMaxPageNum];

  std::fstream& file_;

  ::uint32_t fileSize_;

  ::uint32_t totalPage_;

  void persist(::uint32_t remainedMem);

 public:
  explicit Pager(std::fstream& file);

  ~Pager();

  Byte* getPage(::uint32_t index);

  void close(::uint32_t remainedMem);
};

class Table {
 private:
  Pager* pager_;

  ::uint32_t rowNum_;

  // Table需要自己维护一个递增的索引
  cursor::Cursor* cursor_;

  Byte* getInsertAddr();

  const ::uint32_t kRowCountPerPage = kPageSize / Row::getSize();

 public:
  explicit Table(std::fstream& file);

  ~Table();

  auto getRowNum() const { return this->rowNum_; }

  void insert(const Row& row);

  friend std::ostream& operator<<(std::ostream& os, const Table& table);
};

std::ostream& operator<<(std::ostream& os, const Table& table);

}  // namespace memory

#endif  // TABLE_H__ ends
