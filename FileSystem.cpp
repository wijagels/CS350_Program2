/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "FileSystem.hpp"

#include <fstream>
#include <sstream>
#include <cassert>
#include <string>
#include <vector>

#include "./debug.h"

/*
 * Default constructor, uses default parameters as specified in program
 * description
 */
FileSystem::FileSystem()
    : FileSystem(32, 1024 * 1024, 1024, 10 * 1024, 128 * 1024, 40) {}

FileSystem::FileSystem(uint segment_count, uint segment_size, uint block_size,
                       uint max_files, uint max_file_size, uint imap_blocks)
    : SEGMENT_COUNT(segment_count),
      SEGMENT_SIZE(segment_size),
      BLOCK_SIZE(block_size),
      MAX_FILES(max_files),
      MAX_FILE_SIZE(max_file_size),
      IMAP_BLOCKS(imap_blocks),
      imap_(),
      free_segs_(),
      segment_(),
      dir_() {
  std::ifstream checkpoint("DRIVE/CHECKPOINT_REGION", std::ios::binary);
  assert(checkpoint.is_open());
  checkpoint.seekg(40*4, std::ios::beg);

  for (unsigned i = 0; checkpoint.good() && i < 32; i++) {
    // Get byte
    char buf;
    checkpoint.read(&buf, 1);
    // Read update free table
    free_segs_[i] = buf;
  }

  checkpoint.close();
}

bool FileSystem::import(std::string linux_file, std::string lfs_file) {
  assert(!imap_.is_full());

  std::ifstream file(linux_file, std::ios::binary);
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
  std::vector<std::vector<char> > blocks(size / BLOCK_SIZE + 1);
  logd("Length %lu", blocks.size());
  for (unsigned i = 0; i < size / BLOCK_SIZE + 1; i++) {
    if (BLOCK_SIZE * (i + 1) >= size) {
      logd("Final block %d, end %d", BLOCK_SIZE * i, (unsigned)size);
      blocks[i].insert(blocks[i].end(), &buf[BLOCK_SIZE * i],
                       &buf[(unsigned)size]);
    } else {
      logd("Start %d, end %d", BLOCK_SIZE * i, BLOCK_SIZE * (i + 1));
      blocks[i].insert(blocks[i].end(), &buf[BLOCK_SIZE * i],
                       &buf[BLOCK_SIZE * (i + 1)]);
    }
  }

  // Create a new inode
  Inode node(lfs_file);
  // For each block from lin_fn
  for (unsigned i = 0; i < blocks.size(); i++) {
    // Get a block from the log and write to it
    logd("Writing %lu bytes", blocks[i].size());
    // I hesitate to use reinterpret cast in this situation but we'll work it
    // out later
    auto b_loc = log(reinterpret_cast<char *>(&blocks[i][0]));
    // Store the blocks in the inode
    node[i] = b_loc;
  }

  // Then write the inode to a block
  auto n_loc = log(node);
  // Store the inode in the imap
  auto m_loc = imap_.next_inode();
  imap_[m_loc] = n_loc;
  // Update the imap and update the checkpoint region
  log_imap_sector(4 * m_loc / BLOCK_SIZE);
  // Add the inode to the directory listing
  dir_.add_file(lfs_file, n_loc);

  /* For testing purposes */
  std::ofstream fout;
  fout.open("img.jpg", std::ios::binary);
  for (auto e : blocks) {
    logd("Writing %lu bytes", e.size());
    fout.write(reinterpret_cast<char *>(&e[0]), e.size());
  }
  fout.close();
  return true;
}

bool FileSystem::remove(std::string) { return true; }

std::string FileSystem::list() {
  std::stringstream ss;
  ss << "== List of Files ==" << std::endl;
  ss << "Filename\tinode" << std::endl;
  for (auto e : dir_.dump_inodes()) {
    logd("Read inode %u", e);
    Inode i(imap_[e]);
    ss << i.filename() << "\t" << e << std::endl;
  }
  return ss.str();
}

bool FileSystem::exit() { return true; }

/* Assumes block is of size 1024 */
// TODO Move this to a better place so it can be used by every object in the FS
void fs_read_block(char *block, uint block_num) {
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

int FileSystem::log(char *data) {
  if (!segment_->is_free()) {
    // Find a new free segment
    // Maybe replace with a std::find
    for (uint s = 0; s < free_segs_.size(); s++) {
      if (free_segs_[s]) {
        // Set it to being used
        free_segs_[s] = false;

        // Write segment to drive
        segment_->commit();

        // Get a new segment
        segment_ = SegmentPtr(
            new Segment(s + 1, SEGMENT_SIZE / BLOCK_SIZE, BLOCK_SIZE));
      }
    }
    assert(segment_->is_free());
  }

  auto blk_num = segment_->write(data);
  return (segment_->id() - 1) * SEGMENT_SIZE + blk_num;
}

int FileSystem::log(const Inode &inode) {
  const unsigned filename_len = inode.filename().size() + 1;
  const unsigned data_len = 128 * 4;
  assert(filename_len + data_len <= BLOCK_SIZE);

  char data[BLOCK_SIZE];
  const char *filename = inode.filename().c_str();

  // Populate first part with filename
  uint i;
  for (i = 0; i < filename_len; i++) {
    data[i] = filename[i];
  }
  // Write the data blocks after filename
  for (uint j = 0; j < data_len; j += 4) {
    // Little endian FTW
    data[i + j] = static_cast<char>(inode[j]);
    data[i + j + 1] = static_cast<char>(inode[j] >> 8);
    data[i + j + 2] = static_cast<char>(inode[j] >> 16);
    data[i + j + 3] = static_cast<char>(inode[j] >> 24);
  }
  // Pad rest with 0s
  for (uint k = filename_len + data_len; k < BLOCK_SIZE; k++) {
    data[k] = 0;
  }

  return log(data);
}

void FileSystem::log_imap_sector(uint sector) {
  auto start = sector * BLOCK_SIZE;
  char data[BLOCK_SIZE];
  // Write the imap piece to a block
  for (uint i = 0; i < BLOCK_SIZE / 4; i++) {
    // Little endian FTW
    data[4 * i] = static_cast<char>(imap_[start + i]);
    data[4 * i + 1] = static_cast<char>(imap_[start + i] >> 8);
    data[4 * i + 2] = static_cast<char>(imap_[start + i] >> 16);
    data[4 * i + 3] = static_cast<char>(imap_[start + i] >> 24);
  }
  uint m_loc = log(data);
  char m_loc_bytes[4];
  m_loc_bytes[0] = static_cast<char>(m_loc);
  m_loc_bytes[1] = static_cast<char>(m_loc >> 8);
  m_loc_bytes[2] = static_cast<char>(m_loc >> 16);
  m_loc_bytes[3] = static_cast<char>(m_loc >> 24);

  // Update the checkpoint region
  std::ofstream checkpoint("DRIVE/CHECKPOINT_REGION", std::ios::binary);

  checkpoint.seekp(sector * 4, std::ios::beg);
  checkpoint.write(m_loc_bytes, 4);

  checkpoint.close();
}

unsigned bytes_to_uint(char *bytes) {
  unsigned x = bytes[3];
  x = (x << 8) + bytes[2];
  x = (x << 8) + bytes[1];
  x = (x << 8) + bytes[0];
  return x;
}
