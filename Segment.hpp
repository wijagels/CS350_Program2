/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <vector>

#include "Imap.hpp"

class Segment {
 public:
  using Block = std::vector<char>;

  struct MetaBlock {
    enum class Kind { IMAP, INODE, FILE };
    Kind kind;
    unsigned loc;
    Block block;

    MetaBlock() = delete;
    MetaBlock(Kind k, unsigned l, const Block& b): kind(k), loc(l), block(b) {}
  };

  Segment() = delete;
  Segment(unsigned, unsigned, unsigned);
  inline bool is_free() { return free_block_ < blocks_.size(); }
  unsigned write(char *);
  inline int id() { return id_; }
  void commit();
  void add_file(unsigned, unsigned);
  void remove_file(unsigned);
  std::vector<MetaBlock> clean(const Imap &);

 private:
  void write_uint(char *, unsigned);
  std::vector<Block> blocks_;
  unsigned free_block_;
  unsigned id_;
};
