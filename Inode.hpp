#ifndef INODE_HPP_
#define INODE_HPP_

using Blockid = unsigned;

class Inode {
public:
  Inode();
private:
  std::string fname_;
  std::array<Blockid, 128> blocks_;
};

#endif
