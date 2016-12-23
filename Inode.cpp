/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "Inode.hpp"

#include <string>
#include <cassert>
#include <sstream>

#include "./debug.h"
#include "FileSystem.hpp"

Inode::Inode(Blockid id) : blocks_() {
  blocks_.fill(0);
  // read file info from block #id

  // 129 = 128 block + 1 filesize
  constexpr const unsigned DATA_SZ = 129 * 4;
  char block[1024];
  fs_read_block(block, id);

  // read filesize
  fsize_ = bytes_to_uint(&(block[0]));

  std::stringstream ss;
  // read filename
  unsigned i;
  for (i = 4; i < 1024 - DATA_SZ && block[i] != 0; i++) {
    ss << block[i];
  }
  fname_ = ss.str();
  ++i;

  // make sure we're not out of room
  assert(i < 1024 - DATA_SZ);

  // read the rest
  for (size_t j = 0; j < blocks_.size(); j++) {
    blocks_[j] = bytes_to_uint(&(block[i + j * 4]));
  }
}

Inode::Inode(const std::string& fname, unsigned fsize)
    : fname_{fname}, fsize_{fsize}, blocks_{} {}

Blockid& Inode::operator[](int i) { return blocks_[i]; }

const Blockid& Inode::operator[](int i) const { return blocks_[i]; }

bool Inode::has_block(Blockid id) const {
  for (auto it = blocks_.begin(); it != blocks_.end(); it++) {
    if (*it == id) return true;
  }
  return false;
}

size_t Inode::size() const { return blocks_.size(); }
