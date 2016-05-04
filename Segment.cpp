#include "Segment.hpp"

#include <assert.h>

#include <fstream>
#include <sstream>

Segment::Segment(int id, unsigned blocks, unsigned block_sz):
  blocks_{blocks},
  free_block_{8} {
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
}

Block& Segment::write(char *data) {
  assert(free_block_ < blocks_.size());

  unsigned sz = blocks_[free_block_].size();
  for (unsigned i = 0; i < sz; i++) {
    blocks_[free_block_][i] = data[i];
  }
  return blocks_[free_block_++];
}
