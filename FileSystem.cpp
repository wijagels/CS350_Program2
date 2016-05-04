#include "FileSystem.hpp"

#include <fstream>
#include <sstream>
#include <cassert>
#include <string>

bool FileSystem::import(std::string, std::string) { return true; }

bool FileSystem::remove(std::string) { return true; }

std::string FileSystem::list() { return ""; }

bool FileSystem::exit() { return true; }

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
