#pragma once

#include "def.h"
#include "btree.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

namespace compiler {

const std::string requireCheck = "please check your input and try again!";

template<class... Ts> struct overloaded: Ts... {
  using Ts::operator()...;
};
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

struct MetaCommand {
  std::string_view input_;

  MetaCommand(std::string_view input): input_(input) {}
};

struct SelectSql {
  SelectSql(std::string_view input) {}
};

struct InsertSql {
  memory::Row row;

  InsertSql(std::string_view input) {
    sscanf(
      input.data(), 
      "insert %d %s %s", 
      &row.id, row.name, row.email);
  }
};

using InputType = std::variant<
  MetaCommand, SelectSql, 
  InsertSql, StatusCode
>;

// 工厂模式
class Parser {
  friend class CompilerFactory;

 public:
  static InputType parse(std::string_view input);
};

// 访问者模式
class Interpreter {
  friend class CompilerFactory;

 public:
  StatusCode execute(InputType& input);
};

// 单例模式
class CompilerFactory {
 public:
  static Interpreter& getInterpreter();

  static Parser& getParser();

  static std::tuple<Parser, Interpreter> getAll();
};

}  // namespace compiler
