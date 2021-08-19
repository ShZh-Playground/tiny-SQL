#include<string>
#include<variant>
#include<iostream>

#include"../include/def.h"
#include"../include/compiler.h"

using compiler::CompilerFactory;

void handle_status(StatusCode& statusCode) {
  switch (statusCode) {
    case StatusCode::kSuccess:
      break;
    case StatusCode::kSuccessAndExit:
      std::cout << "Bye~" << std::endl;
      exit(static_cast<int>(StatusCode::kSuccessAndExit));
      break;
    case StatusCode::kUnrecognizedMetaCommand:
      std::cerr << "Error: unrecognized meta command, " 
                << compiler::requireCheck << std::endl;
      break;
    case StatusCode::kUnrecognizedSqlStatement:
      std::cerr << "Error: unrecognized SQL statement, " 
                << compiler::requireCheck << std::endl;
      break;
    default:
      std::cerr << "Fatal Error: Unknown error!" << std::endl;
      exit(static_cast<int>(StatusCode::kUnknownError));
      break;
  }
}

int main() {
  auto [parser, interpreter] = CompilerFactory::GetAll();

  while (true) {
    std::string input;
    std::cout << "TinyDB > ";
    std::getline(std::cin, input);

    auto statement = parser.Parse(input);
    auto statusCode = interpreter.Execute(statement);
    handle_status(statusCode);
  }

  return 0;
}
