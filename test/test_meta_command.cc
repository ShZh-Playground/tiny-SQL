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

TEST(CommandTest, TestExit) {
  // Redirect iostream to string stream
  testing::internal::CaptureStdout();
  auto metaCommand = parser.parse(".exit");
  auto exitStatus = interpreter.execute(metaCommand);
  ASSERT_EQ(exitStatus, StatusCode::kSuccessAndExit);
}

TEST(CommandTest, TestIllegal) {
  // Redirect iostream to string stream
  testing::internal::CaptureStderr();
  auto illegalCommand = parser.parse(".HelloWorld");
  auto illegalStatus = interpreter.execute(illegalCommand);
  ASSERT_EQ(illegalStatus, StatusCode::kUnrecognizeMetaCommand);
  handleStatus(illegalStatus);

  std::string output = testing::internal::GetCapturedStderr();
  ASSERT_STREQ("Error: unrecognized meta command, please check your input and try again!\n", output.c_str());
}

int main() {
  ::testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}
