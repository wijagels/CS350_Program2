#include "Imap.hpp"

#include <assert.h>

#include <fstream>

#include "debug.h"
#include "FileSystem.hpp"

Imap::Imap(): map_{} {
  std::ifstream checkpoint("DRIVE/CHECKPOINT_REGION", std::ios::binary);
  assert(checkpoint.is_open());

  for (unsigned i = 0; checkpoint.good() && i < 40; i++) {
    // Get block where imap part is stored
    unsigned id;
    char buf[4];
    checkpoint.read(buf, 4);
    id = bytes_to_uint(buf);

    // Read segment into memory
    char block[1024];
    fs_read_block(block, id);

    // Read block into imap
    for (int j = 0; j < 256; j++) {
      map_[i*256+j] = bytes_to_uint(&(block[j*4]));
    }
  }

  checkpoint.close();
}

unsigned& Imap::operator[](int i) {
  return map_[i];
}

bool Imap::is_full() {
  int i;
  for (i = 0; i < 10240; i++) {
    if (map_[i] == 0) break;
  }
  return i >= 10240;
}

void Imap::add_inode(unsigned node_blk) {
  for (int i = 0; i < 10240; i++) {
    if (map_[i] == 0) map_[i] = node_blk;
  }
}
