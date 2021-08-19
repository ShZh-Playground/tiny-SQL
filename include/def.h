#pragma once

#include<cstdint>
#include<variant>

// Type alias
using u8  = ::uint8_t;
using u16 = ::uint16_t;
using u32 = ::uint32_t;
using usize = ::uint32_t;
using u64 = ::uint64_t;

using i8 = ::int8_t;
using i16 = ::int16_t;
using i32 = ::int32_t;
using isize = ::int32_t;
using i64 = ::int64_t;

using Byte = char;
using Addr = char*;

// Error definition
enum class StatusCode : u32 {
  kSuccess = 0,
  kSuccessAndExit = 1,

  kUnrecognizedMetaCommand = 100,
  kUnrecognizedSqlStatement = 101,

  kWrongFileSize = 200,
  kDuplicatedKey = 201,

  kInsertError = 300,
  kPageFullError = 301,

  kUnknownError = 900,
};
