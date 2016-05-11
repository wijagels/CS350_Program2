/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <array>
#include <string>

using Blockid = unsigned int;

class Inode {
 public:
  Inode() = delete;
  Inode(const std::string&, unsigned);
  Inode(Blockid);
  Blockid& operator[](int);
  const Blockid& operator[](int) const;
  inline const std::string& filename() const { return fname_; }
  inline int filesize() const { return fsize_; }
  bool has_block(Blockid) const;
  size_t size() const;

 private:
  std::string fname_;
  unsigned fsize_;
  std::array<Blockid, 128> blocks_;
};
