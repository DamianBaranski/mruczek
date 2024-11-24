#ifndef DECODER
#define DECODER
#include <stdint.h>

class Decoder {
  public:
  virtual void fillBuffer(int16_t* buffer) = 0;
  virtual void rewind() = 0;
};

#endif