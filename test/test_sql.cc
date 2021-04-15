#include <gtest/gtest.h>
#include <iostream>
#include <string>
 
#include "../include/compiler.h"
#include "../include/table.h"

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
  std::string insertOutput = testing::internal::GetCapturedStdout();

  // Test select SQL statement
  auto selectStatement = parser.parse("select");
  interpreter.execute(selectStatement);

  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ("Insert OK\n( 1, shzh, shzh7@gmail.com )", output.c_str());
}

int main() {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
