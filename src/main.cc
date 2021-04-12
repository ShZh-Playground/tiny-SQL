#include<iostream>
#include<string>

#include"../include/compiler.h"

int main() {
  auto parser = compiler::CompilerFactory::getParser();
  auto interpreter = compiler::CompilerFactory::getInterpreter();

  while (true) {
    std::string input;
    std::cout << "TinyDB > ";
    std::getline(std::cin, input);

    auto statement = compiler::Parser::parse(input);
    if (statement == nullptr) {
      continue;
    }

    interpreter.execute(statement.get());
  }

  return 0;
}