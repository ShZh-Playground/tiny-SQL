#pragma once

#include<cstdint>
#include<variant>

// Type alias
using u8  = ::uint8_t;
using u16 = ::uint16_t;
using u32 = ::uint32_t;

using Byte = char;
using Addr = char*;

// Error defination
enum Error {
  kNoError = 0,

  kWrongFileSize = 1000,
  kDuplicatedKey = 1001
};
