/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once
#include <string>

using uint = unsigned int;

void fs_read_block(char *, uint);
uint bytes_to_uint(char *);

class FileSystem {
 public:
  FileSystem() : FileSystem(32, 1024 * 1024, 1024, 10 * 1000, 128 * 1024, 40) {}
  FileSystem(uint segment_count, uint segment_size, uint block_size,
             uint max_files, uint max_file_size, uint imap_blocks)
      : SEGMENT_COUNT(segment_count),
        SEGMENT_SIZE(segment_size),
        BLOCK_SIZE(block_size),
        MAX_FILES(max_files),
        MAX_FILE_SIZE(max_file_size),
        IMAP_BLOCKS(imap_blocks) {}
  bool import(std::string, std::string);
  bool remove(std::string);
  std::string cat(std::string);
  std::string display(std::string, uint, uint);
  bool overwrite(std::string, uint, uint, char);
  std::string list();
  bool exit();

 protected:
  const uint SEGMENT_COUNT;
  const uint SEGMENT_SIZE;
  const uint BLOCK_SIZE;
  const uint MAX_FILES;
  const uint MAX_FILE_SIZE;
  const uint IMAP_BLOCKS;
};
