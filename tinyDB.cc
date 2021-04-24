#include<string>
#include<variant>
#include<iostream>

#include"../include/def.h"
#include"../include/compiler.h"

using compiler::CompilerFactory;

void handleStatus(StatusCode statusCode) {
  switch (statusCode) {
    case StatusCode::kSuccess:
      break;
    case StatusCode::kSuccessAndExit:
      std::cout << "Bye~" << std::endl;
      exit(0);
      break;
    case StatusCode::kUnrecognizeMetaCommand:
      std::cerr << "Error: unrecognized meta command, " 
                << compiler::requireCheck << std::endl;
      break;
    case StatusCode::kUnrecognizeSqlStatement:
      std::cerr << "Error: unrecognized SQL statement, " 
                << compiler::requireCheck << std::endl;
      break;
    default:
      break;
  }
}

int main() {
  auto [parser, interpreter] = CompilerFactory::getAll();

  while (true) {
    std::string input;
    std::cout << "TinyDB > ";
    std::getline(std::cin, input);

    auto statement = compiler::Parser::parse(input);
    try {
      // Handle parse error
      handleStatus(std::get<StatusCode>(statement));
    } catch(const std::bad_variant_access&) {
      // Do interprete
      auto statusCode = interpreter.execute(
        std::move(std::get<std::unique_ptr<compiler::CmdInput>>(statement))
      );
      // Handle interprete error
      handleStatus(statusCode);
    } catch (...) {
      std::cerr << "Fatal Error: Unknown error!" << std::endl;
      exit(StatusCode::kUnknownError);
    }
  }

  return 0;
}
