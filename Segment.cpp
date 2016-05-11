/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#include "Segment.hpp"

#include <cassert>
#include <fstream>
#include <sstream>

#include "./debug.h"

#include "FileSystem.hpp"
#include "Inode.hpp"

Segment::Segment(unsigned id, unsigned blocks, unsigned block_sz)
    : blocks_{}, free_block_{8}, id_{id} {
  // Initialize with new blocks
  for (unsigned i = 0; i < blocks; i++) {
    blocks_.push_back(Block(block_sz, '\0'));
  }

  // Get ready to read from a segment file
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << id;
  logd("%s", ss.str().c_str());
  std::ifstream seg_file(ss.str(), std::ios::binary);
  assert(seg_file.is_open());
  seg_file.seekg(0, std::ios::end);
  logd("Size %d", static_cast<int>(seg_file.tellg()));
  seg_file.clear();
  seg_file.seekg(0, std::ios::beg);
  logd("Begin %d", static_cast<int>(seg_file.tellg()));

  // Read seg file
  for (auto& b : blocks_) {
    seg_file.read(b.data(), block_sz);
  }
  seg_file.close();
}

unsigned Segment::write(char* data) {
  assert(free_block_ < blocks_.size());

  unsigned sz = blocks_[free_block_].size();
  for (unsigned i = 0; i < sz; i++) {
    blocks_[free_block_][i] = data[i];
  }

  logd("Wrote to block %u of segment %u", free_block_, id_);

  return free_block_++;
}

void Segment::write_uint(char* b, unsigned x) {
  b[0] = static_cast<char>(x);
  b[1] = static_cast<char>(x >> 8);
  b[2] = static_cast<char>(x >> 16);
  b[3] = static_cast<char>(x >> 24);
}

void Segment::add_file(unsigned inode_id, unsigned block_id) {
  for (unsigned i = 0; i < 8; i++) {
    Block& block = blocks_[i];
    for (unsigned j = 0; j < block.size(); j += 8) {
      if (bytes_to_uint(&block[j + 4]) == 0) {
        write_uint(&block.data()[j], inode_id);
        write_uint(&block.data()[j + 4], block_id);
        return;
      }
    }
  }
  assert(false);
}

void Segment::remove_file(unsigned inode_id) {
  for (unsigned i = 0; i < 8; i++) {
    Block& block = blocks_[i];
    for (unsigned j = 0; j < block.size(); j += 8) {
      assert(bytes_to_uint(&block[j + 4]) != 0);
      if (bytes_to_uint(&block[j]) == inode_id) {
        // TODO
      }
    }
  }
}

void Segment::commit() {
  std::ostringstream ss;
  ss << "DRIVE/SEGMENT" << id_;
  std::ofstream seg_file(ss.str(), std::ios::binary);

  for (auto& b : blocks_) {
    seg_file.write(b.data(), b.size());
  }

  seg_file.close();

  logd("Commit segment %u", id_);
}

std::vector<Segment::MetaBlock> Segment::clean(const Imap &imap) {
  std::vector<MetaBlock> live;

  for (auto summ = blocks_.begin(); summ != blocks_.begin() + 8; summ++) {
    for (size_t i = 0; i < summ->size(); i+=8) {
      unsigned id = bytes_to_uint(&(*summ)[i]);
      unsigned block_n = bytes_to_uint(&(*summ)[i+4]);
      if (block_n != 0) {
        if (id < 10240) {
          // Either an inode block or part of a file
          // Check if imap[id] == block_n => inode block
          //   then copy block to live as inode id
          // Check if inode *id* has block_n in its blocks => file piece
          //   then copy block to live as file assoc with inode id
          Inode inode(id);
          if (imap[id] == block_n) {
            live.push_back(MetaBlock(MetaBlock::Kind::INODE, id, block_n,
                                     blocks_[block_n % blocks_.size()]));
          } else if (inode.has_block(block_n)) {
            live.push_back(MetaBlock(MetaBlock::Kind::FILE, id, block_n,
                                     blocks_[block_n % blocks_.size()]));
          }
        } else {
          // Part of the imap
          // Check if block_n is in the checkpoint region
          // If true, copy block_n to live with imap id
          std::ifstream chkpt("DRIVE/CHECKPOINT_REGION", std::ios::binary);
          assert(chkpt.is_open());

          for (unsigned j = 0; j < 40; j++) {
            char buf[4];
            unsigned mid;
            chkpt.read(buf, 4);
            mid = bytes_to_uint(buf);

            if (mid == block_n) {
              live.push_back(MetaBlock(MetaBlock::Kind::IMAP, j, block_n,
                                       blocks_[block_n % blocks_.size()]));
              break;
            }
          }
          chkpt.close();
        }
      }
    }
  }

  for (auto it = blocks_.begin(); it != blocks_.begin() + 8; it++) {
    *it = Block(it->size(), 0);
  }

  return live;
}
