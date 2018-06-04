#ifndef ONE_CLPIBOARD_H
#define ONE_CLPIBOARD_H

#include <string>

class Clipboard {
public:
  static std::string getString();
  static void setString(std::string value);

  static std::wstring getFilePath();
};

#endif // !ONE_CLPIBOARD_H
