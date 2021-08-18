#include "../include/compiler.h"
#include "../include/btree.h"

#include <string_view>

// 全局变量
extern memory::Table* table;

using compiler::InputType;
using compiler::MetaCommand;
using compiler::SelectSql;
using compiler::InsertSql;

InputType compiler::Parser::parse(std::string_view input) {
  // 以.起始的都不是sql语句，而是内置的命令
  if ('.' == input[0]) {
    return MetaCommand(input);
  }
  // SQL语句
  if ("select" == input.substr(0, 6)) {
    return SelectSql(input);
  }
  if ("insert" == input.substr(0, 6)) {
    return InsertSql(input);
  }

  return StatusCode::kUnrecognizeSqlStatement;
}

StatusCode compiler::Interpreter::execute(InputType& input) {
  return std::visit(compiler::overloaded {
    [] (MetaCommand& command) {
      if (".exit" == command.input_) {
        delete table;
        return StatusCode::kSuccessAndExit;
      }
      if (".btree" == command.input_) {
        structure::print_btree(*table, table->rootIndex_, 0);
        return StatusCode::kSuccess;
      }
      return StatusCode::kUnrecognizeMetaCommand; 
    },
    [] ([[maybe_unused]] SelectSql& sql) {
      std::cout << *table;
      return StatusCode::kSuccess;
    },
    [] (InsertSql& sql) {
      // 选取id作为主键
      table->insert(structure::Pair(sql.row.id, sql.row));
      std::cout << "Insert OK" << std::endl;
      return StatusCode::kSuccess;
    },
    [] (StatusCode& code) { return code; }
  }, input);
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
