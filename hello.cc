#include<iostream>
#include<string>

int main() {
  while (true) {
    std::string input;
    std::cout << "TinyDB>";
    std::getline(std::cin, input);
    if (input == "exit") {
      return 0;
    }
  }
}