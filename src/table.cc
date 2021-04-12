#include"../include/table.h"

using namespace memory;

// 内存中的数据
// 这里只能用全局变量实现
Table table{};

// namespace中的非成员函数要加上namespace来标识
// 不然编译器会认为可能你又定义了一个函数
void memory::saveToMem(const Row& row, Byte* addr) {
  // 不采用默认的对齐方式，内存之间直接紧挨着
  memcpy(addr, &row.id, sizeof(row.id));
  memcpy(addr + sizeof(row.id), row.name, sizeof(row.name));
  memcpy(addr + sizeof(row.id) + sizeof(row.name), row.email, sizeof(row.email));
}

Row memory::loadFromMem(Byte* addr) {
  Row row{};
  memcpy(&(row.id), addr, sizeof(row.id));
  memcpy(row.name, addr + sizeof(row.id), sizeof(row.name));
  memcpy(row.email, addr + sizeof(row.id) + sizeof(row.name), sizeof(row.email));
  return row;
}

Byte* Table::getInsertAddr() {
  int pageIndex = this->index_ / rowCountPerPage;
  int rowOffset = this->index_ % rowCountPerPage;
  if (rowOffset == 0) {
    this->pages_[pageIndex] = new Byte[kPageSize];
  }
  Byte* insertAddr = this->pages_[pageIndex] + rowOffset * Row::getSize();
  
  return insertAddr;
}

void Table::insert(const Row& row) {
  auto insertAddr = this->getInsertAddr();
  saveToMem(row, insertAddr);
  ++this->index_;
}