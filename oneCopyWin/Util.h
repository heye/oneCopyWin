#ifndef ONE_UTIL_H
#define ONE_UTIL_H

#include <string>

class Util {
public:
  static std::string toStr(std::wstring in);
  static std::wstring toWStr(std::string in);
  static std::wstring fileNameFromPath(std::wstring filePath);

  static int getFileSize(std::wstring filePath);
  static void readFile(std::wstring filePath, char* buffer, int bufferSize);

};


#endif