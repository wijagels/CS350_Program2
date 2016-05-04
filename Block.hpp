#pragma once

class Block {
public:
  Block() = delete;
  Block(unsigned);
  virtual ~Block();
  inline char *&block() { return block_; }
  char& operator[](int);
  inline unsigned size() { return sz_; }
private:
  char *block_;
  unsigned sz_;
};
