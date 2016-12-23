/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <array>
#include <string>

using Blockid = unsigned int;

class Inode {
 public:
  Inode() = delete;
  Inode(const std::string&, unsigned);
  explicit Inode(Blockid);
  Blockid& operator[](int index);
  const Blockid& operator[](int index) const;
  inline const std::string& filename() const { return fname_; }
  inline int filesize() const { return fsize_; }
  bool has_block(Blockid) const;
  size_t size() const;

 private:
  std::string fname_;
  unsigned fsize_;
  std::array<Blockid, 128> blocks_;
};
