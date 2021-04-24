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
enum StatusCode : u32 {
  kSuccess = 0,
  kSuccessAndExit = 1,

  kUnrecognizeMetaCommand = 100,
  kUnrecognizeSqlStatement = 101,

  kWrongFileSize = 200,
  kDuplicatedKey = 201,

  kUnknownError = 900
};
