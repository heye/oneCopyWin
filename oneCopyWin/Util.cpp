#include "stdafx.h"
#include "Util.h"

#include <locale>
#include <codecvt>
#include <Shlwapi.h>
#include <fstream>
#include <memory>

std::string Util::toStr(std::wstring in) {
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;

  return converter.to_bytes(in);
}

std::wstring Util::toWStr(std::string in) {
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;

  return converter.from_bytes(in);
}

std::wstring Util::fileNameFromPath(std::wstring filePath) {
  auto fileName = PathFindFileName(filePath.c_str());

  return std::wstring(fileName);
}

int Util::getFileSize(std::wstring filePath) {
  std::ifstream fileForSize(filePath, std::ios::binary);
  auto fsize = fileForSize.tellg();
  fileForSize.seekg(0, std::ios::end);
  fsize = fileForSize.tellg() - fsize;
  fileForSize.close();
  return fsize;
}

void Util::readFile(std::wstring filePath, char* buffer, int bufferSize) {
  std::ifstream fileForRead(filePath, std::ios::in | std::ios::binary);
  fileForRead.read(buffer, bufferSize);
  fileForRead.close();
}
