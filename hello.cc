#include<iostream>
#include<string>

int main() {
  while (true) {
    std::string input;
    std::cout << "TinyDB>";
    std::getline(std::cin, input);
    if (input == "exit") {
      return 0;
    } else {
      std::cout << "Unrecognize command！" << std::endl;
    }
  }
}