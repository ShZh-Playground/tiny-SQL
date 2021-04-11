#ifndef COMPILER_H__
#define COMPILER_H__

#include<tuple>
#include<memory>
#include<string>
#include<cstdlib>
#include<iostream>

namespace compiler {

const std::string requireCheck = "Please check your input and try again!";

class CmdInput {
 protected:
  const std::string input_;

 public:
  CmdInput(const std::string& input): input_(input) {}

  virtual bool execute() = 0;
};

class MetaCommand: public CmdInput {
 public:
  MetaCommand(const std::string& input): CmdInput(input) {}

  bool execute() override {
    if (".exit" == this->input_) {
      std::cout << "Bye" << std::endl;
      std::exit(0);
    } else {
      std::cout << "Unrecognized meta command, " << requireCheck << std::endl;
    }
    return false;
  }
};

class SqlStatement: public CmdInput {
 public:
  SqlStatement(const std::string& input): CmdInput(input) {}

  bool execute() override {
    if ("select" == this->input_.substr(0, 6)) {
      std::cout << "This is where we would do a select." << std::endl;
      return true;
    } else if ("insert" == this->input_.substr(0, 6)) {
      std::cout << "This is where we would do an insert." << std::endl;
      return true;
    } else {
      std::cout << "Unrecognized SQL statement, " << requireCheck << std::endl;
    }
    return false;
  }
};

class CmdInputFactory {
 public:
   static std::shared_ptr<CmdInput>
   getCmdInput(const std::string& input) {
    if (input.size() == 0) {
      return nullptr;
    } else if (input[0] == '.') {   // 以.起始的都不是sql语句，而是内置的命令
      return std::make_shared<MetaCommand>(input);
    } else {                        // sql语句
      return std::make_shared<SqlStatement>(input);
    }
  }
};

} // namespace compiler ends

#endif // COMPILER_H__ marco ends