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

// 前向声明
class Interpreter;

// 访问者模式
class CmdInput {
 protected:
  std::string_view input_;

 public:
  CmdInput(std::string_view input) : input_(input) {}

  virtual ~CmdInput() {}

  std::string_view getInput() const { return this->input_; }

  virtual StatusCode accept(const Interpreter* interpreter) const = 0;
};

class MetaCommand : public CmdInput {
 public:
  MetaCommand(std::string_view input) : CmdInput(input) {}

  StatusCode accept(const Interpreter* interpreter) const override;
};

class SelectSql : public CmdInput {
 public:
  SelectSql(std::string_view input) : CmdInput(input) {}

  StatusCode accept(const Interpreter* interpreter) const override;
};

class InsertSql : public CmdInput {
 private:
  memory::Row row{};

 public:
  InsertSql(std::string_view input) : CmdInput(input) {
    sscanf(input.data(), "insert %d %s %s", &row.id, row.name, row.email);
  }

  memory::Row getRow() const { return this->row; }

  StatusCode accept(const Interpreter* interpreter) const override;
};

// 工厂模式
class Parser {
  friend class CompilerFactory;

 public:
  static std::variant<std::unique_ptr<compiler::CmdInput>, StatusCode> 
  parse(std::string_view input);
};

class Interpreter {
  friend class CompilerFactory;

 private:
  StatusCode visit(std::unique_ptr<CmdInput> cmdInput) const;

 public:
  StatusCode execute(std::unique_ptr<CmdInput> cmdInput) {
    return this->visit(std::move(cmdInput));
  }

  StatusCode visitMetaCommand(const MetaCommand* metaCommand) const;

  StatusCode visitSelectSql(const SelectSql* sqlStatement) const;

  StatusCode visitInsertSql(const InsertSql* sqlStatement) const;
};

// 单例模式
class CompilerFactory {
 public:
  static Interpreter& getInterpreter();

  static Parser& getParser();

  static std::tuple<Parser, Interpreter> getAll();
};

}  // namespace compiler
