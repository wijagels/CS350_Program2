/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#pragma once

#include <array>

class Imap {
 public:
  Imap();
  unsigned& operator[](int index);
  const unsigned& operator[](int index) const;
  bool is_full();
  unsigned next_inode();

 private:
  std::array<unsigned, 10240> map_;
};
