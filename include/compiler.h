#ifndef COMPILER_H__
#define COMPILER_H__

#include<tuple>
#include<memory>
#include<string>
#include<cstdlib>
#include<iostream>

namespace compiler {

const std::string requireCheck = "Please check your input and try again!";

// 前向声明
class Interpreter;

// 访问者模式
class CmdInput {
 protected:
  const std::string input_;

 public:
  CmdInput(const std::string& input): input_(input) {}

  const std::string& getInput() const { return this->input_; }

  virtual bool accept(const Interpreter* interpreter) const = 0;
};

class MetaCommand: public CmdInput {
 public:
  MetaCommand(const std::string& input): CmdInput(input) {}

  bool accept(const Interpreter* interpreter) const override;
};

class SqlStatement: public CmdInput {
 public:
  SqlStatement(const std::string& input): CmdInput(input) {}

  bool accept(const Interpreter* interpreter) const override;
};

// 单例模式
class Parser {
  friend class CompilerFactory;

 public:
   static std::shared_ptr<CmdInput> parse(const std::string& input);
};

class Interpreter {
  friend class CompilerFactory;

 private:
  void visit(const std::shared_ptr<CmdInput> cmdInput);

 public:
  void execute(const std::shared_ptr<CmdInput> cmdInput) { this->visit(cmdInput); }

  bool visitMetaCommand(const MetaCommand* metaCommand) const;

  bool visitSqlStatement(const SqlStatement* sqlStatement) const;
};

class CompilerFactory {
 public:
  static Interpreter& getInterpreter();

  static Parser& getParser();

  static std::tuple<Interpreter, Parser> getAll();
};

} // namespace compiler ends

#endif // COMPILER_H__ marco ends