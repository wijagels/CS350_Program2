#include "Imap.hpp"
#include "debug.h"

#include <fstream>
#include <assert.h>
#include <sstream>

/* Assumes block is of size 1024 */
// TODO Move this to a better place so it can be used by every object in the FS
void fs_read_block(char *block, unsigned block_num) {
  unsigned seg_num = block_num / 1024;
  unsigned seg_ind = block_num % 1024;
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << seg_num;
  std::ifstream seg(ss.str(), std::ios::binary);
  assert(seg.is_open());

  seg.seekg(seg_ind, std::ios::beg);
  seg.read(block, 1024);
}

unsigned bytes_to_uint(char *bytes) {
  unsigned x = bytes[3];
  x = (x << 8) + bytes[2];
  x = (x << 8) + bytes[1];
  x = (x << 8) + bytes[0];
  return x;
}

Imap::Imap() {
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
