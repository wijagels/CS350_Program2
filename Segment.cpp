/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "Segment.hpp"

#include <cassert>
#include <fstream>
#include <sstream>

#include "./debug.h"

#include "FileSystem.hpp"

Segment::Segment(unsigned id, unsigned blocks, unsigned block_sz)
    : blocks_{}, free_block_{8}, id_{id} {
  // Initialize with new blocks
  for (unsigned i = 0; i < blocks; i++) {
    blocks_.push_back(Block(block_sz, '\0'));
  }

  // Get ready to read from a segment file
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << id;
  logd("%s", ss.str().c_str());
  std::ifstream seg_file(ss.str(), std::ios::binary);
  assert(seg_file.is_open());
  seg_file.seekg(0, std::ios::end);
  logd("Size %d", static_cast<int>(seg_file.tellg()));
  seg_file.clear();
  seg_file.seekg(0, std::ios::beg);
  logd("Begin %d", static_cast<int>(seg_file.tellg()));

  // Read seg file
  for (auto& b : blocks_) {
    seg_file.read(b.data(), block_sz);
  }
  seg_file.close();
}

unsigned Segment::write(char* data) {
  assert(free_block_ < blocks_.size());

  unsigned sz = blocks_[free_block_].size();
  for (unsigned i = 0; i < sz; i++) {
    blocks_[free_block_][i] = data[i];
  }

  logd("Wrote to block %u of segment %u", free_block_, id_);

  return free_block_++;
}

void Segment::write_uint(char* b, unsigned x) {
  b[0] = static_cast<char>(x);
  b[1] = static_cast<char>(x >> 8);
  b[2] = static_cast<char>(x >> 16);
  b[3] = static_cast<char>(x >> 24);
}

void Segment::add_file(unsigned inode_id, unsigned block_id) {
  for (unsigned i = 0; i < 8; i++) {
    Block& block = blocks_[i];
    for (unsigned j = 0; j < block.size(); j += 8) {
      if (bytes_to_uint(&block[j + 4]) == 0) {
        write_uint(&block.data()[j], inode_id);
        write_uint(&block.data()[j + 4], block_id);
        return;
      }
    }
  }
  assert(false);
}

void Segment::remove_file(unsigned inode_id) {
  for (unsigned i = 0; i < 8; i++) {
    Block& block = blocks_[i];
    for (unsigned j = 0; j < block.size(); j += 8) {
      assert(bytes_to_uint(&block[j + 4]) != 0);
      if (bytes_to_uint(&block[j]) == inode_id) {
        // TODO
      }
    }
  }
}

void Segment::commit() {
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << id_;
  std::ofstream seg_file(ss.str(), std::ios::binary);

  for (auto& b : blocks_) {
    seg_file.write(b.data(), b.size());
  }

  seg_file.close();

  logd("Commit segment %u", id_);
}
