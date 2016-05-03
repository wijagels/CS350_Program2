#ifndef IMAP_HPP_
#define IMAP_HPP_

#include <array>

using Inodeid = unsigned int;

class Imap {
public:
  Imap();
private:
  std::array<Inodeid, 10240> map_;
};

#endif /* IMAP_H */
