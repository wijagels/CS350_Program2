/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#ifndef INODE_HPP_
#define INODE_HPP_

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
  inline const int filesize() const { return fsize_; }

 private:
  std::string fname_;
  unsigned fsize_;
  std::array<Blockid, 128> blocks_;
};

#endif
