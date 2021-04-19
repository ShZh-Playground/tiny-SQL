#include<iostream>
#include<string>

#include"../include/compiler.h"

using compiler::CompilerFactory;

int main() {
  auto parser = CompilerFactory::getParser();
  auto interpreter = CompilerFactory::getInterpreter();

  while (true) {
    std::string input;
    std::cout << "TinyDB > ";
    std::getline(std::cin, input);

    auto statement = compiler::Parser::parse(input);
    if (statement == nullptr) {
      continue;
    }

    interpreter.execute(std::move(statement));
  }

  return 0;
}
