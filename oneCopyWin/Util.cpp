#include "stdafx.h"
#include "Util.h"

#include <locale>
#include <codecvt>

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
