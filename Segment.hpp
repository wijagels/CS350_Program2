/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <vector>

class Segment {
  using Block = std::vector<char>;

 public:
  Segment() = delete;
  Segment(unsigned, unsigned, unsigned);
  inline bool is_free() { return free_block_ < blocks_.size(); }
  unsigned write(char *);
  inline int id() { return id_; }
  void commit();
  void add_file(unsigned, unsigned);

private:
  void write_uint(char *, unsigned);

 private:
  std::vector<Block> blocks_;
  unsigned free_block_;
  unsigned id_;
};
