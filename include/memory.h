#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include "btree.h"
#include "def.h"
#include "memory.h"

namespace memory {

// 和大多数操作系统分页的大小一样，4096B
constexpr u32 kPageSize = 4096;
constexpr u32 kMaxPageNum = 50;
// 让每一页的数据尽可能少，来测试我们的btree
constexpr u32 kNameMaxLength = 240;
constexpr u32 kEmailMaxLength = 600;

u32 static GetFileSize(std::fstream& file);

#pragma pack(push)
#pragma pack(1)
struct Row {
  u32 id;
  char name[kNameMaxLength];
  char email[kEmailMaxLength];

  friend std::ostream& operator<<(std::ostream& os, const Row& row);
};
#pragma pack(pop)

class Pager {
 private:
  Addr pages[kMaxPageNum];
  std::fstream& file;
  usize file_size;
  usize total_page;

  void Persist();

 public:
  explicit Pager(std::fstream& file);

  ~Pager() noexcept;

  Addr GetPage(usize index);

  // 返回的是未被使用的页面的索引
  usize GetUnusedPage();
};

class Cursor;

struct Table {
  usize root_index;  // 用根节点来表示是哪一个树
  Pager pager;      // 控制页面（节点）

  explicit Table(std::fstream& file);

  template <typename Record>
  void insert(const Record&);

  Cursor GetStart();

  friend std::ostream& operator<<(std::ostream& os, Table& table);
};

std::ostream& operator<<(std::ostream& os, Table& table);

std::ostream& operator<<(std::ostream& os, const Row& row);

struct Cursor {
  Table* table;
  usize page_index;
  usize cell_index;

  // 遍历叶子结点用
  // 如果叶子节点cell的总数为0，说明end_of_table了
  bool end_of_table;

  Cursor() = default;
  Cursor(Table* table, usize page_index, usize cell_index, bool end_of_table)
      : table(table),
        page_index(page_index),
        cell_index(cell_index),
        end_of_table(end_of_table) {}
  ~Cursor() = default;

  void Advance();

  // 前置++
  Cursor operator++();

  // 后置++
  Cursor operator++(int);
};

}  // namespace memory
