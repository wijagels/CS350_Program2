#ifndef IMAP_HPP_
#define IMAP_HPP_

#include <array>

class Imap {
public:
  Imap();
  unsigned& operator[](int);
  bool is_full();
  unsigned& next_inode_id();
private:
  std::array<unsigned, 10240> map_;
};

#endif /* IMAP_H */
