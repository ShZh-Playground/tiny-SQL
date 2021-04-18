#ifndef CURSOR_H__
#define CURSOR_H__

#include<cstdint>

#include"memory.h"

// 前向声明
namespace memory { class Table; }

namespace cursor {

class Cursor {
 private:
  memory::Table& table_;
  ::uint32_t rowIndex_;
  bool isEnd_;

 public:
  Cursor(memory::Table& table, ::uint32_t rowIndex);
  
  [[nodiscard]] auto getCurIndex() const { return this->rowIndex_; }
  
  bool isEnd() const { return this->isEnd_; }

  // For select
  void setBegin();

  // For insert
  void setEnd();

  void advance();

  // 前置++
  Cursor operator++();

  // 后置++
  Cursor operator++(int);
};

} // namespace cursor ends

#endif // CURSOR_H__ ends
