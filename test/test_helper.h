#pragma once

#include<iostream>

#include "../include/def.h"
#include "../include/compiler.h"

void handleStatus(StatusCode& statusCode) {
  switch (statusCode) {
    case StatusCode::kSuccess:
      break;
    case StatusCode::kSuccessAndExit:
      std::cout << "Bye~" << std::endl;
      ::exit(static_cast<u8>(StatusCode::kSuccessAndExit));
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
      std::cerr << "Fatal Error: Unknown error!" << std::endl;
      ::exit(static_cast<u8>(StatusCode::kUnknownError));
      break;
  }
}
