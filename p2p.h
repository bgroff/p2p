#ifndef P2P_H_
#define P2P_H_

enum CODE {
  CONTROL = 0xCC,
  DATA = 0xDD,
  LISTING = 0x4c,
  REQUEST = 0x52,
  TRY = 0x54
};

enum STATE {
  NOT_FOUND = -2,
  ERROR = -1,
  OK = 1
};

struct Packet {
  void *data;
  int count;
};

#endif
  
