#ifndef ONE_UTIL_H
#define ONE_UTIL_H

#include <string>

class Util {
public:
  static std::string toStr(std::wstring in);
  static std::wstring toWStr(std::string in);
};


#endif