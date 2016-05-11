/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <array>
#include <memory>
#include <string>

#include "Imap.hpp"
#include "Inode.hpp"
#include "Segment.hpp"
#include "Directory.hpp"

using uint = unsigned int;

void fs_read_block(char *, uint);
uint bytes_to_uint(char *);

class FileSystem {
 public:
  FileSystem();
  FileSystem(uint segment_count, uint segment_size, uint block_size,
             uint max_files, uint max_file_size, uint imap_blocks);
  bool import(std::string, std::string);
  bool remove(std::string);
  std::string cat(std::string);
  std::string display(std::string, uint, uint);
  bool overwrite(std::string, uint, uint, char);
  std::string list();
  bool clean(unsigned);
  bool exit();

 private:
  int log(char *);
  int log(const Inode &);
  void log_imap_sector(uint);

 private:
  const uint SEGMENT_COUNT;
  const uint SEGMENT_SIZE;
  const uint BLOCK_SIZE;
  const uint MAX_FILES;
  const uint MAX_FILE_SIZE;
  const uint IMAP_BLOCKS;

  using SegmentPtr = std::unique_ptr<Segment>;
  Imap imap_;
  std::array<bool, 32> live_segs_;
  SegmentPtr segment_;
  Directory dir_;
};
