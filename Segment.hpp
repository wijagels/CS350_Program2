#pragma once

#include <vector>

#include "Block.hpp"

class Segment {
public:
  Segment() = delete;
  Segment(int, unsigned, unsigned);
  inline bool is_free() { return free_block_ < blocks_.size(); }
  Block& write(char *);
private:
  std::vector<Block> blocks_;
  unsigned free_block_;
};
