#ifndef TABLE_H__
#define TABLE_H__

#include "btree.h"
#include "memory.h"

#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

namespace memory {

using Byte = char;

// 和大多数操作系统分页的大小一样，4096B
constexpr ::uint32_t kPageSize        = 4096;
constexpr ::uint32_t kMaxPageNum      = 50;
constexpr ::uint32_t kNameMaxLength   = 20;
constexpr ::uint32_t kEmailMaxLength  = 50;

static ::uint32_t getFileSize(std::fstream& file);

template<typename T>
void saveToMemory(void* addr, T obj) {
  ::memcpy(addr, std::addressof(obj), sizeof(T));
}

template<typename T>
T loadFromMemory(void* addr) {
  T ret {};
  ::memcpy(std::addressof(ret), addr, sizeof(T));
  return ret;
}

#pragma pack(push)
#pragma pack(1)
struct Row {
  ::uint32_t id;

  char name[kNameMaxLength];

  char email[kEmailMaxLength];

  constexpr static ::uint32_t getSize() {
    return sizeof(id) + sizeof(name) + sizeof(email);
  }

  friend std::ostream& operator<<(std::ostream& os, const Row& row);
};
#pragma pack(pop)

class Cursor {
 private:
  ::uint32_t pageIndex_;

  ::uint32_t cellIndex_;

 public:
  Cursor(::uint32_t pageIndex_, ::uint32_t cellIndex_);

  [[nodiscard]] auto getPageIndex() const { return this->pageIndex_; }

  [[nodiscard]] auto getCellIndex() const { return this->cellIndex_; }

  void advance();

  // 前置++
  Cursor operator++();

  // 后置++
  Cursor operator++(int);
};

class Pager {
 private:
  Byte* pages_[kMaxPageNum];

  std::fstream& file_;

  ::uint32_t fileSize_;

  ::uint32_t totalPage_;

  void persist();

 public:
  explicit Pager(std::fstream& file);

  ~Pager() noexcept;

  Byte* getPage(::uint32_t index);
};

class Table {
 private:
  // 控制页面（节点）
  Pager* pager_;

  // 控制节点中的记录
  Cursor* cursor_;

  // 用根节点来表示是哪一个树
  ::uint32_t rootIndex_;

 public:
  explicit Table(std::fstream& file);

  ~Table() noexcept;

  template<typename Record>
  void insert(const Record&);

  friend std::ostream& operator<<(std::ostream& os, const Table& table);
};

std::ostream& operator<<(std::ostream& os, const Table& table);

std::ostream& operator<<(std::ostream& os, const Row& row);

}  // namespace memory

#endif  // TABLE_H__ ends
