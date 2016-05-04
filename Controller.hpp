/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "FileSystem.hpp"

enum exitstatus { OKAY = 0, NOT_CMD = 1, BAD_LEN = 2, FS_ERROR = 3, EXIT = 4 };

class Controller {
 public:
  Controller() : Controller(std::cin, std::cout) {}
  Controller(std::istream &in, std::ostream &out) : input_(in), output_(out) {}
  bool parse_commands();
  exitstatus execute_command(std::string);

 protected:
  std::istream &input_;
  std::ostream &output_;
  FileSystem fs_;
};

std::vector<std::string> split(std::string, char);
