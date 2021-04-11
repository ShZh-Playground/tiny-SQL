#include<iostream>
#include<string>

#include"../include/compiler.h"

int main() {
  while (true) {
    std::string input;
    std::cout << "TinyDB > ";
    std::getline(std::cin, input);

    auto cmdInput = compiler::CmdInputFactory::getCmdInput(input);
    if (cmdInput == nullptr) {
      continue;
    }

    cmdInput->execute();
  }

  return 0;
}