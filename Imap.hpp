/* Copyright 2016 Sarude Dandstorm $ ORIGINAL MIX */
#ifndef IMAP_HPP_
#define IMAP_HPP_

#include <array>

class Imap {
 public:
  Imap();
  unsigned& operator[](int);
  const unsigned& operator[](int) const;
  bool is_full();
  unsigned next_inode();

 private:
  std::array<unsigned, 10240> map_;
};

#endif /* IMAP_H */
