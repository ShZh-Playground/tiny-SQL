#include <gtest/gtest.h>

#include <string>

#include "test_helper.h"

using compiler::CompilerFactory;

TEST(SqlTest, TestInsert) {
  // Redirect iostream to string stream
  testing::internal::CaptureStdout();

  // Test insert SQL statement
  auto insert_statement =
      compiler::Parser::Parse("insert 1 shzh shzh7@gmail.com");
  auto insert_status = compiler::Interpreter::Execute(insert_statement);
  ASSERT_EQ(insert_status, StatusCode::kSuccess);
  handle_status(insert_status);

  // Test select SQL statement
  auto select_statement = compiler::Parser::Parse("select");
  auto select_status = compiler::Interpreter::Execute(select_statement);
  ASSERT_EQ(select_status, StatusCode::kSuccess);
  handle_status(select_status);

  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ("Insert OK\n( 1, shzh, shzh7@gmail.com )\n", output.c_str());
}

TEST(SqlTest, TestIllegal) {
  // Redirect iostream to string stream
  testing::internal::CaptureStderr();

  auto illegal_statement = compiler::Parser::Parse("HelloWorld");
  auto illegal_status = compiler::Interpreter::Execute(illegal_statement);
  ASSERT_EQ(illegal_status, StatusCode::kUnrecognizedSqlStatement);
  handle_status(illegal_status);

  std::string output = testing::internal::GetCapturedStderr();
  ASSERT_STREQ(
      "Error: unrecognized SQL statement, please check your input and try "
      "again!\n",
      output.c_str());
}

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
