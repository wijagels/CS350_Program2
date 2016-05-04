#include "Block.hpp"

Block::Block(unsigned sz):
  block_{new char[sz]},
  sz_{sz} {}

Block::~Block() {
  delete[] block_;
}

char& Block::operator[](int i) {
  return block_[i];
}
