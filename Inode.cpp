/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "Inode.hpp"

#include <assert.h>

#include "FileSystem.hpp"

Inode::Inode(Blockid id) : blocks_() {
  // read file info from block #id
  constexpr const unsigned DATA_SZ = 128 * 4;
  char block[1024];
  fs_read_block(block, id);
  char fname[1025 - DATA_SZ];

  // read filename
  unsigned i;
  for (i = 0; i < 1024 - DATA_SZ && block[i] != 0; i++) {
    fname[i] = block[i];
  }
  fname[i + 1] = '\0';
  fname_ = std::string{fname};

  // make sure we're not out of room
  assert(i < 1024 - DATA_SZ);

  // read the rest
  for (int j = 0; j < 128; j++) {
    blocks_[j] = bytes_to_uint(&(block[i + j*4]));
  }
}

Inode::Inode(const std::string& fname, unsigned fsize) : fname_{fname}, fsize_{fsize}, blocks_() {}

Blockid& Inode::operator[](int i) { return blocks_[i]; }

const Blockid& Inode::operator[](int i) const { return blocks_[i]; }
