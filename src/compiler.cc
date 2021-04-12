#include"../include/compiler.h"

using namespace compiler;

bool MetaCommand::accept(const Interpreter* interpreter) const {
  return interpreter->visitMetaCommand(this);
}

bool SqlStatement::accept(const Interpreter* interpreter) const {
  return interpreter->visitSqlStatement(this);
}

std::shared_ptr<CmdInput> Parser::parse(const std::string& input) {
  if (input.size() == 0) {
    return nullptr;
  } else if (input[0] == '.') {   // 以.起始的都不是sql语句，而是内置的命令
    return std::make_shared<MetaCommand>(input);
  } else {                        // sql语句
    return std::make_shared<SqlStatement>(input);
  }
}

void Interpreter::visit(const CmdInput* cmdInput) {
  cmdInput->accept(this);
}

bool Interpreter::visitMetaCommand(const MetaCommand* metaCommand) const {
  if (".exit" == metaCommand->getInput()) {
    std::cout << "Bye" << std::endl;
    std::exit(0);
  } else {
    std::cout << "Unrecognized meta command, " << requireCheck << std::endl;
  }
  return false;
}

bool Interpreter::visitSqlStatement(const SqlStatement* sqlStatement) const {
  if ("select" == sqlStatement->getInput().substr(0, 6)) {
    std::cout << "This is where we would do a select." << std::endl;
    return true;
  } else if ("insert" == sqlStatement->getInput().substr(0, 6)) {
    std::cout << "This is where we would do an insert." << std::endl;
    return true;
  } else {
    std::cout << "Unrecognized SQL statement, " << requireCheck << std::endl;
  }
  return false;
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