#pragma once

#include <vector>

#include "Block.hpp"

class Segment {
public:
  Segment() = delete;
  Segment(unsigned, unsigned, unsigned);
  inline bool is_free() { return free_block_ < blocks_.size(); }
  unsigned write(char *);
  inline int id() { return id_; }
private:
  std::vector<Block> blocks_;
  unsigned free_block_;
  unsigned id_;
};