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
  unsigned add_file(std::string, unsigned);
  unsigned lookup_file(std::string);
  unsigned remove_file(std::string);

 protected:
  void write_out(void);
  std::string dir_file_name_;
  std::fstream dir_file_;
  std::map<std::string, unsigned> dir_map_;
};
