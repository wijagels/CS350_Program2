/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <utility>

#include "Imap.hpp"

class Directory {
 public:
  Directory();
  explicit Directory(std::string);
  ~Directory();
  unsigned add_file(std::string, unsigned, unsigned);
  unsigned lookup_file(std::string);
  unsigned remove_file(std::string);
  std::vector<unsigned> dump_inodes();
  std::map<std::string, std::pair<unsigned, unsigned> > get_map() {
    return dir_map_;
  }

 protected:
  void write_out(void);
  std::string dir_file_name_;
  std::fstream dir_file_;

  // < filename <inode num, filesize> >
  std::map<std::string, std::pair<unsigned, unsigned> > dir_map_;
};
