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

TEST(CommandTest, TestExit) {
  // Redirect iostream to string stream
  testing::internal::CaptureStdout();
  auto metaCommand = parser.parse(".exit");
  interpreter.execute(metaCommand);

  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ("Bye\n", output.c_str());
}

TEST(CommandTest, TestIllegal) {
  // Redirect iostream to string stream
  testing::internal::CaptureStdout();
  auto illegalCommand = parser.parse(".HelloWorld");
  interpreter.execute(illegalCommand);

  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_STREQ("Unrecognized meta command, please check your input and try again!\n", output.c_str());
}
