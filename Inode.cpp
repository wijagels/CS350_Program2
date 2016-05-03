#include "Inode.hpp"
#include "FileSystem.hpp"

#include <assert.h>

Inode::Inode(Blockid id): blocks_{} {
  // read file info from block #id
  constexpr const unsigned DATA_SZ = 128*4;
  char block[1024];
  fs_read_block(block, id);
  char fname[1025-DATA_SZ];

  // read filename
  unsigned i = 0;
  while (i < 1024-DATA_SZ && block[i] != 0) {
    fname[i] = block[i];
    i++;
  }
  fname[i+1] = '\0';
  fname_ = std::string{fname};

  // make sure we're not out of room
  assert(i < 1024-DATA_SZ);

  // read the rest
  for (int j = 0; j < 128; j++) {
    assert(i < 1024);
    blocks_[j] = bytes_to_uint(&(block[i]));
    i += 4;
  }
}
