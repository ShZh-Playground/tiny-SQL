#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "test_helper.h"
#include "../include/def.h"
#include "../include/compiler.h"
#include "../include/memory.h"

using compiler::CompilerFactory;
using compiler::Parser;
using compiler::Interpreter;

auto parser = CompilerFactory::getParser();
auto interpreter = CompilerFactory::getInterpreter();

TEST(SqlTest, TestInsert) {
  // Redirect iostream to string stream
  testing::internal::CaptureStdout();
  
  // Test insert SQL statement
  auto insertStatement = parser.parse("insert 1 shzh shzh7@gmail.com");
  auto insertStatus = interpreter.execute(insertStatement);
  ASSERT_EQ(insertStatus, StatusCode::kSuccess);
  handleStatus(insertStatus);

  // Test select SQL statement
  auto selectStatement = parser.parse("select");
  auto selectStatus = interpreter.execute(selectStatement);
  ASSERT_EQ(selectStatus, StatusCode::kSuccess);
  handleStatus(selectStatus);

  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ("Insert OK\n( 1, shzh, shzh7@gmail.com )\n", output.c_str());
}

TEST(SqlTest, TestIllegal) {
  // Redirect iostream to string stream
  testing::internal::CaptureStderr();

  auto illegalStatement = parser.parse("HelloWorld");
  auto illegalStatus = interpreter.execute(illegalStatement);
  ASSERT_EQ(illegalStatus, StatusCode::kUnrecognizeSqlStatement);
  handleStatus(illegalStatus);

  std::string output = testing::internal::GetCapturedStderr();
  ASSERT_STREQ("Error: unrecognized SQL statement, please check your input and try again!\n", output.c_str());
}

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
