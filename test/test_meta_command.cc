#include <gtest/gtest.h>

#include <string>

#include "test_helper.h"

TEST(CommandTest, TestExit) {
  // Redirect iostream to string stream
  testing::internal::CaptureStdout();
  auto meta_command = compiler::Parser::Parse(".exit");
  auto exit_status = compiler::Interpreter::Execute(meta_command);
  ASSERT_EQ(exit_status, StatusCode::kSuccessAndExit);
}

TEST(CommandTest, TestIllegal) {
  // Redirect iostream to string stream
  testing::internal::CaptureStderr();
  auto illegal_command = compiler::Parser::Parse(".HelloWorld");
  auto illegal_status = compiler::Interpreter::Execute(illegal_command);
  ASSERT_EQ(illegal_status, StatusCode::kUnrecognizedMetaCommand);
  handle_status(illegal_status);

  std::string output = testing::internal::GetCapturedStderr();
  ASSERT_STREQ(
      "Error: unrecognized meta command, please check your input and try "
      "again!\n",
      output.c_str());
}

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
