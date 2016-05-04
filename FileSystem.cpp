#include "FileSystem.hpp"

#include <fstream>
#include <sstream>
#include <cassert>
#include <string>

bool FileSystem::import(std::string lin_fn, std::string lfs_fn) {
  assert(!imap_.is_full());

  std::ifstream f(lin_fn, std::ios::binary);
  assert(f.is_open());

  char *blocks[MAX_FILE_SIZE];
  uint i;
  for (i = 0; f.good() && i < MAX_FILE_SIZE/BLOCK_SIZE; i++) {
    blocks[i] = new char[BLOCK_SIZE];
    f.read(blocks[i], BLOCK_SIZE);
  }
  assert(!f.good());

  Inode node(lfs_fn);
  // For each block from lin_fn
  for (uint j = 0; j < i; j++) {
    // Get a block from the log and write to it
    int b_id = log(blocks[j]);
    // Store the blocks in the inode
    node[j] = b_id;
    // Then write the inode to a block
    int n_id = log(node);
    // Store the inode in the imap
    imap_.next_inode_id() = n_id;
  }

  // Cleanup
  for (uint z = 0; z < i; z++) {
    delete[] blocks[z];
  }
}

bool FileSystem::remove(std::string) { return true; }

std::string FileSystem::list() { return ""; }

bool FileSystem::exit() { return true; }

/* Assumes block is of size 1024 */
// TODO Move this to a better place so it can be used by every object in the FS
void read_block(char *block, uint block_num) {
  uint seg_num = block_num / 1024;
  uint seg_ind = block_num % 1024;
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << seg_num;
  std::ifstream seg(ss.str(), std::ios::binary);
  assert(seg.is_open());

  seg.seekg(seg_ind, std::ios::beg);
  seg.read(block, 1024);
  seg.close();
}

unsigned bytes_to_uint(char *bytes) {
  unsigned x = bytes[3];
  x = (x << 8) + bytes[2];
  x = (x << 8) + bytes[1];
  x = (x << 8) + bytes[0];
  return x;
}
