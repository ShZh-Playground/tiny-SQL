#include<cstdlib>

#include"../include/compiler.h"

using namespace compiler;

// 全局变量
extern memory::Table table;

bool MetaCommand::accept(const Interpreter* interpreter) const {
  return interpreter->visitMetaCommand(this);
}

bool SelectSql::accept(const Interpreter* interpreter) const {
  return interpreter->visitSelectSql(this);
}

bool InsertSql::accept(const Interpreter* Interpreter) const {
  return Interpreter->visitInsertSql(this);
}

std::shared_ptr<CmdInput> Parser::parse(const std::string& input) {
  // 不合法的输入
  if (input.size() == 0) {
    return nullptr;
  }
  // 以.起始的都不是sql语句，而是内置的命令
  if ('.' == input[0]) {
    return std::make_shared<MetaCommand>(input);
  }
  // SQL语句
  if ("select" == input.substr(0, 6)) {
    return std::make_shared<SelectSql>(input);
  } else if ("insert" == input.substr(0, 6)) {
    return std::make_shared<InsertSql>(input);
  } else {
    std::cout << "Unrecognized SQL statement, " << requireCheck << std::endl;
  }

  return nullptr;
}

void Interpreter::visit(const std::shared_ptr<CmdInput> cmdInput) {
  cmdInput->accept(this);
}

bool Interpreter::visitMetaCommand(const MetaCommand* metaCommand) const {
  if (".exit" == metaCommand->getInput()) {
    std::cout << "Bye" << std::endl;
    exit(0);
  } else {
    std::cout << "Unrecognized meta command, " << requireCheck << std::endl;
  }
  return false;
}

bool Interpreter::visitSelectSql(const SelectSql* sqlStatement) const {
  std::cout << table;
  return true;
}

bool Interpreter::visitInsertSql(const InsertSql* sqlStatement) const {
  table.insert(sqlStatement->getRow());
  std::cout << "Insert OK" << std::endl;
  return true;
}

Interpreter& CompilerFactory::getInterpreter() {
  static Interpreter interpreter;
  return interpreter;
}

Parser& CompilerFactory::getParser() {
  static Parser parser;
  return parser;
}

std::tuple<Interpreter, Parser> CompilerFactory::getAll() {
  return std::make_tuple(getInterpreter(), getParser());
}