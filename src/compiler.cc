#include "../include/compiler.h"

#include <cstdlib>
#include <string_view>

// 全局变量
extern memory::Table* table;

bool compiler::MetaCommand::accept(const Interpreter* interpreter) const {
  return interpreter->visitMetaCommand(this);
}

bool compiler::SelectSql::accept(const Interpreter* interpreter) const {
  return interpreter->visitSelectSql(this);
}

bool compiler::InsertSql::accept(const Interpreter* Interpreter) const {
  return Interpreter->visitInsertSql(this);
}

std::unique_ptr<compiler::CmdInput> compiler::Parser::parse(
    std::string_view input) {
  // 不合法的输入
  if (input.empty()) {
    return nullptr;
  }
  // 以.起始的都不是sql语句，而是内置的命令
  if ('.' == input[0]) {
    return std::make_unique<MetaCommand>(input);
  }
  // SQL语句
  if ("select" == input.substr(0, 6)) {
    return std::make_unique<SelectSql>(input);
  }
  if ("insert" == input.substr(0, 6)) {
    return std::make_unique<InsertSql>(input);
  }
  std::cout << "Unrecognized SQL statement, " << requireCheck << std::endl;

  return nullptr;
}

void compiler::Interpreter::visit(std::unique_ptr<CmdInput>&& cmdInput) const {
  cmdInput->accept(this);
}

bool compiler::Interpreter::visitMetaCommand(
    const MetaCommand* metaCommand) const {
  if (".exit" == metaCommand->getInput()) {
    delete table;
    std::cout << "Bye" << std::endl;
    exit(0);
  } else {
    std::cout << "Unrecognized meta command, " << requireCheck << std::endl;
  }
  return false;
}

bool compiler::Interpreter::visitSelectSql(
    const SelectSql* sqlStatement) const {
  std::cout << *table;
  return true;
}

bool compiler::Interpreter::visitInsertSql(
    const InsertSql* sqlStatement) const {
  table->insert(sqlStatement->getRow());
  std::cout << "Insert OK" << std::endl;
  return true;
}

compiler::Interpreter& compiler::CompilerFactory::getInterpreter() {
  static compiler::Interpreter interpreter;
  return interpreter;
}

compiler::Parser& compiler::CompilerFactory::getParser() {
  static compiler::Parser parser;
  return parser;
}

std::tuple<compiler::Interpreter, compiler::Parser>
compiler::CompilerFactory::getAll() {
  return std::make_tuple(getInterpreter(), getParser());
}
