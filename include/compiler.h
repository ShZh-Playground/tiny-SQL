#ifndef COMPILER_H__
#define COMPILER_H__

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

#include "memory.h"

namespace compiler {

const std::string requireCheck = "please check your input and try again!";

// 前向声明
class Interpreter;

// 访问者模式
class CmdInput {
 protected:
  const std::string input_;

 public:
  CmdInput(const std::string& input) : input_(input) {}

  const std::string& getInput() const { return this->input_; }

  virtual bool accept(const Interpreter* interpreter) const = 0;
};

class MetaCommand : public CmdInput {
 public:
  MetaCommand(const std::string& input) : CmdInput(input) {}

  bool accept(const Interpreter* interpreter) const override;
};

class SelectSql : public CmdInput {
 public:
  SelectSql(const std::string& input) : CmdInput(input) {}

  bool accept(const Interpreter* interpreter) const override;
};

class InsertSql : public CmdInput {
 private:
  memory::Row row{};

 public:
  InsertSql(const std::string& input) : CmdInput(input) {
    sscanf(input.c_str(), "insert %d %s %s", &row.id, row.name, row.email);
  }

  memory::Row getRow() const { return this->row; }

  bool accept(const Interpreter* interpreter) const override;
};

// 工厂模式
class Parser {
  friend class CompilerFactory;

 public:
  static std::unique_ptr<CmdInput> parse(const std::string& input);
};

class Interpreter {
  friend class CompilerFactory;

 private:
  void visit(std::unique_ptr<CmdInput>&& cmdInput) const;

 public:
  void execute(std::unique_ptr<CmdInput>&& cmdInput) {
    this->visit(std::move(cmdInput));
  }

  bool visitMetaCommand(const MetaCommand* metaCommand) const;

  bool visitSelectSql(const SelectSql* sqlStatement) const;

  bool visitInsertSql(const InsertSql* sqlStatement) const;
};

// 单例模式
class CompilerFactory {
 public:
  static Interpreter& getInterpreter();

  static Parser& getParser();

  static std::tuple<Interpreter, Parser> getAll();
};

}  // namespace compiler

#endif  // COMPILER_H__ marco ends
