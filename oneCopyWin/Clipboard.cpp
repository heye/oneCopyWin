#include "stdafx.h"

#include "Clipboard.h"
#include "Util.h"

#include <Windows.h>

std::string Clipboard::getString() {
  std::wstring strData;

  if (OpenClipboard(NULL))
  {
    HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
    if (hClipboardData)
    {
      WCHAR *pchData = (WCHAR*)GlobalLock(hClipboardData);
      if (pchData)
      {
        strData = pchData;
        GlobalUnlock(hClipboardData);
      }
    }
    CloseClipboard();
  }

  return Util::toStr(strData);
}

void Clipboard::setString(std::string value) {
  if (value.empty()) {
    return;
  }
  auto valueW = Util::toWStr(value);


  OpenClipboard(0);
  EmptyClipboard();
  HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * (wcslen(valueW.c_str()) + 1));
  if (!hg) {
    CloseClipboard();
    return;
  }

  wchar_t* pchData;
  pchData = (wchar_t*)GlobalLock(hg);
  auto size = wcslen(valueW.c_str()) + 1;
  wcscpy_s(pchData, size, valueW.c_str());
  GlobalUnlock(hg);

  SetClipboardData(CF_UNICODETEXT, hg);
  CloseClipboard();
  GlobalFree(hg);
}
