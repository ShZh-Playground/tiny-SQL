#include <gtest/gtest.h>
#include <iostream>
#include <string>
 
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
  interpreter.execute(insertStatement);

  // Test select SQL statement
  auto selectStatement = parser.parse("select");
  interpreter.execute(selectStatement);

  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ("Insert OK\n( 1, shzh, shzh7@gmail.com )\n", output.c_str());
}

TEST(SqlTest, TestIllegal) {
  // Redirect iostream to string stream
  testing::internal::CaptureStdout();
  
  auto illegalStatement = parser.parse("HelloWorld");
  ASSERT_EQ(illegalStatement, nullptr);

  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ("Unrecognized SQL statement, please check your input and try again!\n", output.c_str());
}

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
