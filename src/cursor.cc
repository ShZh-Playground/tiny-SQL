#include "../include/cursor.h"
#include "../include/memory.h"

using cursor::Cursor;

// index指向最后一条record的后一个时候设置end标记
Cursor::Cursor(memory::Table& table, ::uint32_t rowIndex) :
  table_(table),
  rowIndex_(rowIndex),
  isEnd_(rowIndex >= table.getRowNum()) {}

void Cursor::advance() {
  if (this->rowIndex_ < this->table_.getRowNum()) {
    ++this->rowIndex_;
    this->isEnd_ = this->rowIndex_ >= this->table_.getRowNum(); 
  }
}

void Cursor::setBegin() {
  this->rowIndex_ = 0;  
  this->isEnd_ = this->rowIndex_ >= this->table_.getRowNum(); 
}

void Cursor::setEnd() { 
  this->rowIndex_ = this->table_.getRowNum(); 
  this->isEnd_ = true; 
}

Cursor Cursor::operator++() {
  this->advance();
  return *this;
}

Cursor Cursor::operator++(int n) {
  auto ret = *this;
  this->advance();
  return ret;
}
