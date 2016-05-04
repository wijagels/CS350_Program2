#include "Segment.hpp"

#include <assert.h>

#include <fstream>
#include <sstream>

#include "FileSystem.hpp"

Segment::Segment(int id, unsigned blocks, unsigned block_sz):
  blocks_{blocks},
  free_block_{0} {
  // Initialize with new blocks
  for (auto& b: blocks_) {
    b = Block{block_sz};
  }

  // Get ready to read from a segment file
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << id;
  std::ifstream seg_file{ss.str(), std::ios::binary};
  assert(seg_file.is_open());

  // Read seg file
  for (auto& b: blocks_) {
    seg_file.read(b.block(), block_sz);
  }
  seg_file.close();

  // Find the next block to write to
  for (unsigned i = 0; i < 8; i++) {
    Block b = blocks_[i];
    for (unsigned j = 4; j < block_sz; j += 8) {
      // Read the block part of the inode,block pair
      unsigned taken = bytes_to_uint(&(b[j])) % blocks;
      // The goal is to pick the block ahead of the last
      // known taken block
      if (taken >= free_block_) free_block_ = taken+1;
    }
  }
}
