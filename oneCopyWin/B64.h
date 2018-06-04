#ifndef CP_B64
#define CP_B64

#include <string>

class B64 {
private:
  static char encodingTable[];
  static char decodingTable[256];
  static bool init_;
  //static bool isAvailable_;
public:
  static std::string encode(char const* bytes_to_encode, unsigned int in_len);

  /*
  returns -1 on failure
  otherwise returns length of the written buffer including terminating null character at the end of the buffer
  */
  static int decode(std::string inputData, char &outBuff, int outBuffSize);

  /*
  returns the maxium length of data encoded in a b64 of this size
  note: the actually encoded data may be shorter due to padding
  */
  static int b64DecodeSize(int b64StringSize);

  static void init();

  //static bool test();

  //static bool isAvailable();

};


#endif