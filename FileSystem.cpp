/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "FileSystem.hpp"

#include <fstream>
#include <sstream>
#include <cassert>
#include <string>

#include "./debug.h"

/*
 * Default constructor, uses default parameters as specified in program
 * description
 */
FileSystem::FileSystem()
    : FileSystem(32, 1024 * 1024, 1024, 10 * 1000, 128 * 1024, 40) {}

FileSystem::FileSystem(uint segment_count, uint segment_size, uint block_size,
                       uint max_files, uint max_file_size, uint imap_blocks)
    : SEGMENT_COUNT(segment_count),
      SEGMENT_SIZE(segment_size),
      BLOCK_SIZE(block_size),
      MAX_FILES(max_files),
      MAX_FILE_SIZE(max_file_size),
      IMAP_BLOCKS(imap_blocks),
      imap_() {}

bool FileSystem::import(std::string file_name, std::string destination) {
  std::ifstream file;
  file.open(file_name, std::ios::binary);
  assert(file.is_open());
  file.seekg(0, std::ios::end);
  std::streampos size = file.tellg();
  logd("File size %d", static_cast<int>(size));
  if (size > MAX_FILE_SIZE) {
    return false;
  }
  char *buf = new char[size];
  file.seekg(0, std::ios::beg);
  file.read(buf, size);
  file.close();
  return true;
}

bool FileSystem::remove(std::string) { return true; }

std::string FileSystem::list() {
  std::stringstream ss;
  ss << "== List of Files ==";
  return ss.str();
}

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
