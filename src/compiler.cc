#include "../include/compiler.h"

#include <cstdlib>
#include <string_view>

// 全局变量
extern memory::Table* table;

StatusCode compiler::MetaCommand::accept(const Interpreter* interpreter) const {
  return interpreter->visitMetaCommand(this);
}

StatusCode compiler::SelectSql::accept(const Interpreter* interpreter) const {
  return interpreter->visitSelectSql(this);
}

StatusCode compiler::InsertSql::accept(const Interpreter* Interpreter) const {
  return Interpreter->visitInsertSql(this);
}

std::variant<std::unique_ptr<compiler::CmdInput>, StatusCode>
compiler::Parser::parse(std::string_view input) {
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

  return StatusCode::kUnrecognizeSqlStatement;
}

StatusCode compiler::Interpreter::visit(std::unique_ptr<CmdInput> cmdInput) const {
  return cmdInput->accept(this);
}

StatusCode compiler::Interpreter::visitMetaCommand(
    const MetaCommand* metaCommand) const {
  if (".exit" == metaCommand->getInput()) {
    delete table;
    return StatusCode::kSuccessAndExit;
  }
  return StatusCode::kUnrecognizeMetaCommand;
}

StatusCode compiler::Interpreter::visitSelectSql(
    [[maybe_unused]] const SelectSql* sqlStatement) const {
  std::cout << *table;
  return StatusCode::kSuccess;
}

StatusCode compiler::Interpreter::visitInsertSql(
    const InsertSql* sqlStatement) const {
  // 选取id作为主键
  table->insert(structure::Pair(sqlStatement->getRow().id, sqlStatement->getRow()));
  std::cout << "Insert OK" << std::endl;
  return StatusCode::kSuccess;
}

compiler::Interpreter& compiler::CompilerFactory::getInterpreter() {
  static compiler::Interpreter interpreter;
  return interpreter;
}

compiler::Parser& compiler::CompilerFactory::getParser() {
  static compiler::Parser parser;
  return parser;
}

std::tuple<compiler::Parser, compiler::Interpreter>
compiler::CompilerFactory::getAll() {
  return std::make_tuple(getParser(), getInterpreter());
}
