/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "Directory.hpp"

#include <cassert>
#include <string>
#include <fstream>

#include "Controller.hpp"
#include "Imap.hpp"

Directory::Directory() : Directory("DRIVE/DIR") {}

Directory::Directory(std::string file)
    : dir_file_name_(file), dir_file_(file, std::ios::in) {
  assert(dir_file_.is_open());
  std::string line;
  while (getline(dir_file_, line)) {
    auto tokenized = split(line, ':');
    add_file(tokenized.at(0), stoi(tokenized.at(1)));
  }
  dir_file_.close();
}

Directory::~Directory() { write_out(); }

void Directory::write_out(void) {
  /* std::ios::trunc clobbers the file for us */
  dir_file_.open(dir_file_name_, std::ios::out | std::ios::trunc);
  assert(dir_file_.is_open());
  for (auto e : dir_map_) {
    dir_file_ << e.first << ":" << e.second << std::endl;
  }
}

unsigned Directory::add_file(std::string name, unsigned inode) {
  dir_map_.insert({name, inode});
  return inode;
}

unsigned Directory::lookup_file(std::string name) {
  try {
    return dir_map_.at(name);
  } catch (std::out_of_range) {
    return 0;
  }
}

unsigned Directory::remove_file(std::string name) {
  try {
    unsigned deleted = dir_map_.at(name);
    dir_map_.erase(name);
    return deleted;
  } catch (std::out_of_range) {
    return 0;
  }
}
