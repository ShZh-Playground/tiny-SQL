#pragma once

#include "def.h"
#include "btree.h"
#include "memory.h"

#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

namespace memory {
  
// 和大多数操作系统分页的大小一样，4096B
constexpr u32 kPageSize        = 4096;
constexpr u32 kMaxPageNum      = 50;
constexpr u32 kNameMaxLength   = 20;
constexpr u32 kEmailMaxLength  = 50;

static u32 getFileSize(std::fstream& file);

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
  u32   id;
  char  name[kNameMaxLength];
  char  email[kEmailMaxLength];

  constexpr static u32 getSize() {
    return sizeof(id) + sizeof(name) + sizeof(email);
  }

  friend std::ostream& operator<<(std::ostream& os, const Row& row);
};
#pragma pack(pop)

class Cursor {
 private:
  u32 pageIndex_;
  u32 cellIndex_;

 public:
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
  Addr          pages_[kMaxPageNum];
  std::fstream& file_;
  u32           fileSize_;
  u32           totalPage_;

  void persist();

 public:
  explicit Pager(std::fstream& file);

  ~Pager() noexcept;

  Addr getPage(u32 index);
};

class Table {
 private:
  u32     rootIndex_;   // 用根节点来表示是哪一个树
  Pager   pager_;       // 控制页面（节点）
  Cursor  cursor_;      // 控制节点中的记录

 public:
  explicit Table(std::fstream& file);

  template<typename Record>
  void insert(const Record&);

  friend std::ostream& operator<<(std::ostream& os, Table& table);
};

std::ostream& operator<<(std::ostream& os, Table& table);

std::ostream& operator<<(std::ostream& os, const Row& row);

}  // namespace memory
