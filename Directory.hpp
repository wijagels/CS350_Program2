/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <string>
#include <fstream>
#include <map>

#include "Imap.hpp"

class Directory {
 public:
  Directory();
  explicit Directory(std::string);
  ~Directory();
  Inodeid add_file(std::string, Inodeid);
  Inodeid lookup_file(std::string);
  Inodeid remove_file(std::string);

 protected:
  void write_out(void);
  std::string dir_file_name_;
  std::fstream dir_file_;
  std::map<std::string, Inodeid> dir_map_;
};
