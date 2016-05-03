#ifndef INODE_HPP_
#define INODE_HPP_

#include <array>
#include <string>

using Blockid = unsigned int;

class Inode {
public:
  Inode() = delete;
  Inode(Blockid);
  Blockid& operator[](int);
private:
  std::string fname_;
  std::array<Blockid, 128> blocks_;
};

#endif
