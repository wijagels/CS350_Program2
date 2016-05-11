/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "FileSystem.hpp"

#include <fstream>
#include <sstream>
#include <cassert>
#include <string>
#include <vector>
#include <iomanip>

#include "./debug.h"

/*
 * Default constructor, uses default parameters as specified in program
 * description
 */
FileSystem::FileSystem()
    : FileSystem{32, 1024 * 1024, 1024, 10 * 1024, 128 * 1024, 40} {}

FileSystem::FileSystem(uint segment_count, uint segment_size, uint block_size,
                       uint max_files, uint max_file_size, uint imap_blocks)
    : SEGMENT_COUNT{segment_count},
      SEGMENT_SIZE{segment_size},
      BLOCK_SIZE{block_size},
      MAX_FILES{max_files},
      MAX_FILE_SIZE{max_file_size},
      IMAP_BLOCKS{imap_blocks},
      imap_{},
      live_segs_{},
      segment_{},
      dir_{} {
  std::ifstream checkpoint{"DRIVE/CHECKPOINT_REGION", std::ios::binary};
  assert(checkpoint.is_open());
  checkpoint.seekg(160, std::ios::beg);

  for (unsigned i = 0; checkpoint.good() && i < 32; i++) {
    // Get byte
    char buf;
    checkpoint.get(buf);
    // Read update free table
    live_segs_[i] = buf;
  }

  checkpoint.close();

  for (unsigned i = 0; i < live_segs_.size(); i++) {
    if (!live_segs_[i]) {
      segment_ =
          SegmentPtr{new Segment{i, SEGMENT_SIZE / BLOCK_SIZE, BLOCK_SIZE}};
      live_segs_[i] = true;
      break;
    }
  }
}

bool FileSystem::import(std::string linux_file, std::string lfs_file) {
  assert(!imap_.is_full());
  if (imap_.is_full()) {
    loge("Imap is full, please delete some files");
    return false;
  }
  std::ifstream file(linux_file, std::ios::binary);
  assert(file.is_open());
  if (!file.is_open()) {
    loge("Failed to open file: %s", linux_file.c_str());
    return false;
  }
  file.seekg(0, std::ios::end);
  std::streampos size = file.tellg();
  logd("File size %d", static_cast<int>(size));
  if (size > MAX_FILE_SIZE) {
    loge("%u bytes is too large", static_cast<unsigned>(size));
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
      // 0-pad the vector
      blocks[i].resize(BLOCK_SIZE, 0);
    } else {
      logd("Start %d, end %d", BLOCK_SIZE * i, BLOCK_SIZE * (i + 1));
      blocks[i].insert(blocks[i].end(), &buf[BLOCK_SIZE * i],
                       &buf[BLOCK_SIZE * (i + 1)]);
    }
  }

  // Store the inode in the imap
  auto m_loc = imap_.next_inode();

  // Create a new inode
  Inode node(lfs_file, static_cast<unsigned>(size));
  // For each block from lin_fn
  for (unsigned i = 0; i < blocks.size(); i++) {
    // Get a block from the log and write to it
    logd("Writing %lu bytes", blocks[i].size());
    auto b_loc = log(blocks.at(i).data());
    logd("Destination block: [%u]:%u", i, b_loc);
    // Store the blocks in the inode
    node[i] = b_loc;
    segment_->add_file(m_loc, b_loc);
  }

  // Then write the inode to a block
  auto n_loc = log(node);
  imap_[m_loc] = n_loc;
  // Update the imap and update the checkpoint region
  logd("%u", m_loc);
  logd("%u", n_loc);
  log_imap_sector(4 * m_loc / BLOCK_SIZE);
  // Add the inode to the directory listing
  dir_.add_file(lfs_file, m_loc, static_cast<int>(size));
  // Inform the segment of the new file
  segment_->add_file(m_loc, n_loc);

  delete[] buf;
  return true;
}

bool FileSystem::remove(std::string file) {
  unsigned inode = dir_.lookup_file(file);
  if (inode == (unsigned)-1) {
    return false;
  }
  dir_.remove_file(file);
  imap_[inode] = 0;
  log_imap_sector(4 * inode / BLOCK_SIZE);
  return true;
}

/*
 * display <lfs_filename> <howmany> <start>
 *
 * Read and display <howmany> bytes from file <lfs_filename>
 * beginning at logical byte <start>.
 *
 * Display the bytes on the screen.
 */
std::string FileSystem::display(std::string file, uint howmany, uint start) {
  unsigned inode = dir_.lookup_file(file);
  if (inode == (unsigned)-1) {
    return "Not a file!";
  }
  std::string result = cat(file);
  return result.substr(start, howmany);
}

std::string FileSystem::list() {
  const int COL_WIDTH = 20;
  std::stringstream ss;
  ss << std::setw(COL_WIDTH) << "Filename" << std::setw(COL_WIDTH) << "inode"
     << std::setw(COL_WIDTH) << "Filesize" << std::endl;
  ss << std::setfill('-') << std::setw(COL_WIDTH * 3) << "-" << std::endl
     << std::setfill(' ');
  for (const auto &e : dir_.get_map()) {
    ss << std::setw(COL_WIDTH) << e.first << std::setw(COL_WIDTH)
       << e.second.first << std::setw(COL_WIDTH) << e.second.second
       << std::endl;
  }
  ss << std::endl;
  return ss.str();
}

/*
 * clean <n>
 *
 * Where n is the segment number to be cleaned
 *
 */
bool FileSystem::clean(unsigned segments) {
  using Kind = Segment::MetaBlock::Kind;
  assert(1 <= segments && segments <= 32);

  std::fstream checkpoint("DRIVE/CHECKPOINT_REGION", std::ios::binary | std::ios::in |
                          std::ios::out);
  assert(checkpoint.is_open());

  // Commit current segment and load up a clean one
  std::vector<bool> written(32, false);
  segment_->commit();
  for (size_t s = 0; s < live_segs_.size(); s++) {
    if (!live_segs_[s]) {
      // Set it to being used
      live_segs_[s] = true;

      // Write segment to drive
      segment_->commit();

      // Get a new segment
      segment_ =
        SegmentPtr(new Segment(s, SEGMENT_SIZE / BLOCK_SIZE, BLOCK_SIZE));
    }
  }
  written[segment_->id()] = true;

  unsigned cleaned = 0;
  for (size_t s = 0; cleaned < segments && s < live_segs_.size(); s++) {
    if (!live_segs_[s] || s == static_cast<unsigned>(segment_->id()) || written[s]) {
      continue;
    }

    Segment to_clean(s, SEGMENT_SIZE / BLOCK_SIZE, BLOCK_SIZE);

    // Clean and get live data, write to disk
    auto live_data = to_clean.clean(imap_);
    to_clean.commit();
    live_segs_[s] = false;

    unsigned t;
    Inode *inode;
    // Go through live data and make updates appropriately
    for (auto &meta: live_data) {
      switch (meta.kind) {
      case Kind::FILE:
        // Go to inode id and change the old block
        // Log meta.block, inode, and imap sector
        // WARN Write new imap sector and then logging IMAP sectors later on in this
        // list will cause inaccuracies
        inode = new Inode(imap_[meta.id]);

        t = log(meta.block.data());
        for (size_t i = 0; i < inode->size(); i++) {
          if ((*inode)[i] == meta.loc) {
            (*inode)[i] = t;
            break;
          }
        }

        t = log(*inode);
        imap_[meta.id] = t;
        log_imap_sector(4 * meta.id / BLOCK_SIZE);

        delete inode;
        inode = nullptr;
        break;
      case Kind::INODE:
        // Log block
        // Go to imap[id] and change entry to logged block
        // Log imap sector
        t = log(meta.block.data());
        imap_[meta.id] = t;
        log_imap_sector(4 * meta.id / BLOCK_SIZE);
        break;
      case Kind::IMAP:
        // Log block and update checkpoint region
        // Lazy af
        log_imap_sector(meta.id);
        break;
      default:
        assert(false);
        break;
      }
    }
    written[segment_->id()] = true;
    cleaned++;
  }

  checkpoint.close();

  return true;
}

bool FileSystem::exit() {
  segment_->commit();  // Commits to disk, works %100$ i promise chelsea

  // Write segment use to checkpoint region
  std::fstream checkpoint{"DRIVE/CHECKPOINT_REGION",
                          std::ios::binary | std::ios::in | std::ios::out};
  checkpoint.seekp(160, std::ios::beg);

  for (auto s : live_segs_) {
    // logd("Writing segment availability: %d", s);
    checkpoint.put(s);
  }

  checkpoint.close();
  return true;
}

std::string FileSystem::cat(std::string filename) {
  segment_->commit();
  unsigned inum = dir_.lookup_file(filename);
  assert(inum != (unsigned)-1);
  if (inum == (unsigned)-1) {
    loge("File does not exist!");
    return "";
  }
  unsigned blockid = imap_[inum];
  logd("Getting inode %u:%u", inum, blockid);
  assert(blockid != (unsigned)-1);
  if (blockid == (unsigned)-1) {
    loge("Inode points to invalid block");
    return "";
  }
  Inode inode{blockid};
  std::stringstream ss;
  char block[1025];
  for (size_t i = 0; i < 128; i++) {
    // logd("Access block [%zu]:%u", i, inode[i]);
    fs_read_block(block, inode[i]);
    block[1024] = '\0';
    // logd("block %s", block);
    ss << block;
  }
  logd("%s", inode.filename().c_str());
  logd("%u", inode.filesize());
  ss << std::endl;
  return ss.str();
}

/* Assumes block is of size 1024 */
// TODO Move this to a better place so it can be used by every object in the FS
void fs_read_block(char *block, uint block_num) {
  uint seg_num = block_num / 1024;
  uint seg_ind = block_num % 1024;
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << seg_num;
  logd("Reading block %u from segment %u starting at byte %u in file %s",
       block_num, seg_num, seg_ind * 1024, ss.str().c_str());
  std::ifstream seg(ss.str(), std::ios::binary);
  assert(seg.is_open());
  if (!seg.is_open()) {
    loge("Segment is not open");
    throw;
  }

  seg.seekg(seg_ind * 1024, std::ios::beg);
  seg.read(block, 1024);
  seg.close();
}

int FileSystem::log(char *data) {
  assert(segment_.get() != nullptr);
  if (!segment_->is_free()) {
    // Find a new free segment
    // Maybe replace with a std::find
    for (uint s = 0; s < live_segs_.size(); s++) {
      if (!live_segs_[s]) {
        // Set it to being used
        live_segs_[s] = true;

        // Write segment to drive
        segment_->commit();

        // Get a new segment
        segment_ =
            SegmentPtr(new Segment(s, SEGMENT_SIZE / BLOCK_SIZE, BLOCK_SIZE));
      }
    }
    assert(segment_->is_free());
  }

  auto blk_num = segment_->write(data);
  return segment_->id() * SEGMENT_SIZE / BLOCK_SIZE + blk_num;
}

int FileSystem::log(const Inode &inode) {
  const unsigned filename_len = inode.filename().size() + 1;
  const unsigned data_len = 128 * 4;
  assert(filename_len + data_len <= BLOCK_SIZE);

  char data[BLOCK_SIZE];
  std::fill(data, data + BLOCK_SIZE, 0);
  const char *filename = inode.filename().c_str();

  // Populate first 4 bytes with filesize
  data[0] = static_cast<char>(inode.filesize());
  data[1] = static_cast<char>(inode.filesize() >> 8);
  data[2] = static_cast<char>(inode.filesize() >> 16);
  data[3] = static_cast<char>(inode.filesize() >> 24);
  uint iter = 4;

  // Populate second part with filename
  for (uint i = 0; i < filename_len; i++) {
    data[iter] = filename[i];
    iter++;
  }
  data[iter] = '\0';
  // Write the data blocks after filename
  for (uint j = 0; j < data_len / 4; j++) {
    logd("writing out %u", inode[j]);
    // Little endian FTW
    data[iter + j * 4] = static_cast<char>(inode[j]);
    data[iter + j * 4 + 1] = static_cast<char>(inode[j] >> 8);
    data[iter + j * 4 + 2] = static_cast<char>(inode[j] >> 16);
    data[iter + j * 4 + 3] = static_cast<char>(inode[j] >> 24);
  }
  // Pad rest with 0s
  // for (uint k = filename_len + data_len; k < BLOCK_SIZE; k++) {
  //   data[k] = 0;
  // }

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
  segment_->add_file(-1, m_loc);
  logd("New imap segment written to block %u", m_loc);
  char m_loc_bytes[4];
  m_loc_bytes[0] = static_cast<char>(m_loc);
  m_loc_bytes[1] = static_cast<char>(m_loc >> 8);
  m_loc_bytes[2] = static_cast<char>(m_loc >> 16);
  m_loc_bytes[3] = static_cast<char>(m_loc >> 24);

  // Update the checkpoint region
  std::ofstream checkpoint("DRIVE/CHECKPOINT_REGION",
                           std::ios::binary | std::ios::out | std::ios::in);

  logd("Sector: %d", sector);
  checkpoint.seekp(sector * 4, std::ios::beg);
  logd("Logging to byte: %d", static_cast<int>(checkpoint.tellp()));
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
