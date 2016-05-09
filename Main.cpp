/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "Controller.hpp"
#include "FileSystem.hpp"
#include "Initializer.hpp"
#include "Directory.hpp"

int main(int argc, char** argv) {
  if (argc != 1 && argc != 2) {
    std::cerr << argv[0] << std::endl;
    std::cerr << "Invalid number of arguments" << std::endl;
    exit(1);
  } else if (argc == 2) {
    if (std::string(argv[1]) == "initialize") {
      initialize_fs();
      return 0;
    } else {
      std::cerr << "Invalid number of arguments" << std::endl;
      exit(1);
    }
  }
  Controller c;
  return c.parse_commands() == true ? 0 : 1;
}

bool Controller::parse_commands() {
  bool status = true;
  while (!input_.eof()) {
    std::string cmd;
    std::getline(input_, cmd);
    if (input_.eof()) return status;
    exitstatus result = execute_command(cmd);
    switch (result) {
      case OKAY:
        break;
      case NOT_CMD:
        std::cerr << "Not a command!" << std::endl;
        status = false;
        break;
      case BAD_LEN:
        std::cerr << "Invalid number of arguments" << std::endl;
        status = false;
        break;
      case FS_ERROR:
        std::cerr << "Filesystem Erorr" << std::endl;
        break;
      case EXIT:
        return status;
    }
  }
  return status;
}

// TODO(will): add support for other commands when they're added to FS
exitstatus Controller::execute_command(std::string cmd) {
  std::vector<std::string> tokenized = split(cmd, ' ');
  if (tokenized.size() == 0) return BAD_LEN;
  if (tokenized.at(0) == "import") {
    if (tokenized.size() != 3) return BAD_LEN;
    bool status = fs_.import(tokenized.at(1), tokenized.at(2));
    if (status) return OKAY;
    return FS_ERROR;
  }
  if (tokenized.at(0) == "remove") {
    if (tokenized.size() != 2) return BAD_LEN;
    bool status = fs_.remove(tokenized.at(1));
    if (status) return OKAY;
  }
  if (tokenized.at(0) == "list" || tokenized.at(0) == "ls") {
    if (tokenized.size() != 1) return BAD_LEN;
    std::string output = fs_.list();
    output_ << output << std::endl;
    return OKAY;
  }
  if (tokenized.at(0) == "exit") {
    if (tokenized.size() != 1) return BAD_LEN;
    bool status = fs_.exit();
    if (status) return EXIT;
    return FS_ERROR;
  }
  if (tokenized.at(0) == "ilookup") {
    if (tokenized.size() != 2) return BAD_LEN;
    Directory d;
    std::cout << d.lookup_file(tokenized.at(1)) << std::endl;
    return OKAY;
  }
  return NOT_CMD;
}

/* http://code.runnable.com/VHb0hWMZp-ws1gAr/splitting-a-string-into-a-vector-for-c
 */
std::vector<std::string> split(std::string str, char delimiter) {
  std::vector<std::string> internal;
  std::stringstream ss(str);  // Turn the string into a stream.
  std::string tok;
  while (getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }
  return internal;
}
