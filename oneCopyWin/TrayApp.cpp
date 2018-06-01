
#include "stdafx.h"
#include "TrayApp.h"

#include <Shlwapi.h>
#include <psapi.h>

#include <locale>
#include <codecvt>
#include <sstream>
#include <memory>

#include "resource.h"

#include "PocoRequest.h"
#include "B64.h"

#ifdef UNICODE
#define stringcopy wcscpy_s
#else
#define stringcopy strcpy
#endif

#define ID_TRAY_APP_ICON                355633
#define WM_TRAYICON ( WM_USER + 1 )
#define TRAY_QUIT_MSG ( WM_USER + 2 )



int cplan::TrayApp::run(HINSTANCE instance) {
  instance_ = instance;

  createWindow();

  cplan::TrayApp::update("started app!", "oneCopy");

  //run main application
  MSG msg;
  BOOL bret = 0;
  while ((bret = GetMessage(&msg, window_, 0, 0)) != 0)
  {
    if (bret == -1)
    {
      break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}


cplan::TrayApp::TrayApp() :
  window_(nullptr),
  iconDEFAULT_(nullptr),
  addedIcon_(false),
  contextMenu_(nullptr)
{

}

cplan::TrayApp::~TrayApp()
{
  DestroyMenu(contextMenu_);
  DestroyWindow(window_);
  removeIcon();
}

cplan::TrayApp & cplan::TrayApp::getInstance() {
  static cplan::TrayApp app;
  return app;
}

HWND cplan::TrayApp::createWindow()
{
  iconDEFAULT_ = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_TRAY));
  WNDCLASSEX wnd = { 0 };
  wnd.hInstance = instance_;
  TCHAR className[] = TEXT("tray icon class");
  wnd.lpszClassName = className;
  wnd.lpfnWndProc = windowCB;
  wnd.style = CS_HREDRAW | CS_VREDRAW;
  wnd.cbSize = sizeof(WNDCLASSEX);
  wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
  wnd.hbrBackground = (HBRUSH)COLOR_APPWORKSPACE;
  if (!RegisterClassEx(&wnd))
  {
    return nullptr;
  }

  //create a window for handling of the tray-events
  //we will never show this window
  window_ = CreateWindow(
    className,
    TEXT("Using the system tray"),
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT,
    400, 400,
    NULL, NULL,
    instance_, NULL
  );
  memset(&notificationData_, 0, sizeof(NOTIFYICONDATA));
  notificationData_.cbSize = sizeof(NOTIFYICONDATA);
  notificationData_.hWnd = window_;
  notificationData_.uID = ID_TRAY_APP_ICON;
  notificationData_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; // when someone clicks on the system tray icon,
                                                               // we want a WM_ type message to be sent to our WNDPROC
  notificationData_.dwInfoFlags = NIIF_INFO;
  notificationData_.uTimeout = 10;
  notificationData_.uCallbackMessage = WM_TRAYICON; //this message must be handled in hwnd's window procedure. more info below.
  notificationData_.uVersion = NOTIFYICON_VERSION_4;
}

HMENU cplan::TrayApp::createContext()
{
  //initialize menu
  contextMenu_ = CreatePopupMenu();
  addMenuItem("push", std::bind(&cplan::TrayApp::pushAction));
  //addMenuSpacer();
  addMenuItem("pull", std::bind(&cplan::TrayApp::pullAction));
  addMenuSpacer();
  addMenuItem("settings", std::bind(&cplan::TrayApp::settingsAction));
  addMenuItem("about", std::bind(&cplan::TrayApp::aboutAction));
  addMenuItem("exit", std::bind(&cplan::TrayApp::doQuit, this));
  return contextMenu_;
}

// Message handler for about box.
INT_PTR CALLBACK About2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG: {
    std::wstring apikey = L"test-key";
    SetDlgItemText(hDlg, IDC_EDIT1, apikey.c_str());

    return (INT_PTR)TRUE;
  }

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK)
    {
      EndDialog(hDlg, LOWORD(wParam));

      WCHAR apikey[256];
      GetDlgItemText(hDlg, IDC_EDIT1, apikey, 256);

      WCHAR server[256];
      GetDlgItemText(hDlg, IDC_EDIT2, server, 256);

      return (INT_PTR)TRUE;
    }
    if (LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}


LRESULT CALLBACK cplan::TrayApp::windowCB(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  //wParam: try icon id

  switch (message)
  {
  case WM_COMMAND:
  {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId)
    {
    case IDM_ABOUT:
      DialogBox(cplan::TrayApp::getInstance().instance_, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About2);
      break;
    case IDM_SETTINGS:
      DialogBox(cplan::TrayApp::getInstance().instance_, MAKEINTRESOURCE(IDD_SETTINGS), hwnd, About2);
      break;
    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
    }
  }
  break;
  case WM_CREATE:
    TrayApp::getInstance().createContext();

    break;
  case WM_TRAYICON:
  {
    ////printf( "Tray icon notification");

    // the mouse button has been released.
    if (lParam == WM_LBUTTONUP || lParam == WM_LBUTTONDBLCLK)
    {
      //printf( "Button up\n");
      //cplan::TrayApp::showFolder();
      //clicked on icon
    }
    else if (lParam == WM_RBUTTONDOWN || lParam == WM_RBUTTONDBLCLK) // I'm using WM_RBUTTONDOWN here because
    {
      //printf( "Button down\n");

      // Get current mouse position.
      POINT curPoint;
      GetCursorPos(&curPoint);

      SetForegroundWindow(hwnd);

      // TrackPopupMenu blocks the app until TrackPopupMenu returns
      if (getInstance().contextMenu_) {
        UINT clicked = TrackPopupMenu(
          getInstance().contextMenu_,
          TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD | TPM_NONOTIFY, // don't send me WM_COMMAND messages about this window, instead return the identifier of the clicked menu item
          curPoint.x,
          curPoint.y,
          0,
          hwnd,
          NULL);

        for (auto && oneElement : getInstance().callbacks_) {
          if (clicked == oneElement.first) {

            if (oneElement.second) {
              oneElement.second();
            }
          }
        }
      }
    }
  }
  break;
  case TRAY_QUIT_MSG:
    //catch quit message from other thread?! works...
    PostQuitMessage(0);

    break;
    break;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

void cplan::TrayApp::removeIcon() {
  Shell_NotifyIcon(NIM_DELETE, &(getInstance().notificationData_));
}







void cplan::TrayApp::addMenuItem(std::string title, buttonCallback callback) {
  if (!contextMenu_) {
    return;
  }

  static int callbackID = 0;
  callbackID++;

  callbacks_.emplace(callbackID, callback);

  AppendMenuA(contextMenu_, MF_STRING, callbackID, title.c_str());
}

void cplan::TrayApp::addMenuSpacer() {
  if (!contextMenu_) {
    return;
  }

  AppendMenu(contextMenu_, MF_MENUBREAK, 0, nullptr);
}


void cplan::TrayApp::update(std::string title, std::string message) {

  // receive update from pipe , but window is not yet created, or other
  if (NULL == window_)
  {
    return;
  }

  if (!title.empty() && !message.empty()) {

    //TODO: set message  & title
    std::wstring titleW(title.length(), L' ');
    std::copy(title.begin(), title.end(), titleW.begin());
    stringcopy(notificationData_.szInfo, titleW.c_str());

    std::wstring messageW(message.length(), L' ');
    std::copy(message.begin(), message.end(), messageW.begin());
    stringcopy(notificationData_.szInfoTitle, messageW.c_str());
    //stringcopy(notificationData_.szInfoTitle, TEXT("balloooooon!"));
  }

  if (!tooltip_.empty()) {
    //TODO: set tooltip
    // set the tooltip text.  must be LESS THAN 64 chars
    std::wstring tmpStr(tooltip_.length(), L' ');
    std::copy(tooltip_.begin(), tooltip_.end(), tmpStr.begin());
    tooltip_.clear();
    stringcopy(notificationData_.szTip, tmpStr.c_str());
  }

  notificationData_.hIcon = iconDEFAULT_;

  // add the icon to the system tray
  if (!addedIcon_) {
    addedIcon_ = Shell_NotifyIcon(NIM_ADD, &notificationData_);
    if (!addedIcon_)
    {
      Sleep(200);
      addedIcon_ = Shell_NotifyIcon(NIM_ADD, &notificationData_);
    }
  }
  else {
    Shell_NotifyIcon(NIM_MODIFY, &notificationData_);
  }
}

void cplan::TrayApp::showNotification(std::string title, std::string message) {
  getInstance().update(title, message);
}

void cplan::TrayApp::setTooltip(std::string tooltip) {
  getInstance().tooltip_ = tooltip;
  getInstance().update();
}

void cplan::TrayApp::exit() {
  //this will cause GetMessage to return false
  //PostQuitMessage(0);
  printf("send quit message \n");
  auto success = PostMessage(cplan::TrayApp::getInstance().window_, WM_QUIT, 0, 0);

  if (!success) {
    printf("send quit message failed %d\n", GetLastError());
  }
  else {
    printf("send quit message success\n");
  }

}

void cplan::TrayApp::pushAction() {
  auto value = fromClipboard();

  auto valueEncoded = CPNet::B64::encode(value.c_str(), value.length());

  std::stringstream json;
  json << "{\"type\":\"set_key\",\"key\":\"test-key\",\"value\":\"" << valueEncoded << "\"}";

  CPNet::PocoRequest request("https://azenix.io");
  auto reply = request.post(json.str());

}

void cplan::TrayApp::pullAction() {

  //get from server
  CPNet::PocoRequest request("https://azenix.io");

  auto reply = request.post("{\"type\":\"get_key\",\"key\":\"test-key\"}");

  auto val = request.getString("value");
  
  //decode base64
  std::unique_ptr<char[]> data(new char[CPNet::B64::b64DecodeSize(val.length())]);

  auto dataSize = CPNet::B64::decode(val, *data.get(), CPNet::B64::b64DecodeSize(val.length())) - 1;
  if (dataSize < 0) {
    return;
  }

  std::string valStr(data.get(), dataSize);

  //set clipboard value
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;

  cplan::TrayApp::toClipboard(converter.from_bytes(valStr));
}

void cplan::TrayApp::settingsAction() {
  //ShellExecuteA(NULL, "open", "http://127.0.0.1:56392", NULL, NULL, SW_SHOWNORMAL);
  cplan::TrayApp::showNotification("settings!", "oneCopy");

  PostMessage(cplan::TrayApp::getInstance().window_, WM_COMMAND, IDM_SETTINGS, 0);
}

void cplan::TrayApp::aboutAction() {
  //ShellExecuteA(NULL, "open", "http://127.0.0.1:56392", NULL, NULL, SW_SHOWNORMAL);
  //cplan::TrayApp::showNotification("settings!", "oneCopy");

  PostMessage(cplan::TrayApp::getInstance().window_, WM_COMMAND, IDM_ABOUT, 0);
  //settingsWindow::getInstance().about();
}

void cplan::TrayApp::doQuit() {
  //?
}


void cplan::TrayApp::toClipboard(const std::wstring &wstr) {
  if (wstr.empty()) {
    return;
  }

  OpenClipboard(0);
  EmptyClipboard();

  HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * (wcslen(wstr.c_str()) + 1));
  if (!hg) {
    CloseClipboard();
    return;
  }

  wchar_t* pchData;
  pchData = (wchar_t*)GlobalLock(hg);
  auto size = wcslen(wstr.c_str()) + 1;
  wcscpy_s(pchData, size, wstr.c_str());
  GlobalUnlock(hg);

  SetClipboardData(CF_UNICODETEXT, hg);
  CloseClipboard();
  GlobalFree(hg);

}

std::string cplan::TrayApp::fromClipboard() {

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


  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;
  return converter.to_bytes(strData);
}


/*void cplan::TrayApp::toClipboard(const std::wstring &wstr) {
  if (!wstr.empty()) {

    OpenClipboard(0);
    EmptyClipboard();

    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t) * (wcslen(wstr.c_str()) + 1));
    if (!hg) {
      CloseClipboard();
      return;
    }

    wchar_t* pchData;
    pchData = (wchar_t*)GlobalLock(hg);
    auto size = wcslen(wstr.c_str()) + 1;
    wcscpy_s(pchData, size,wstr.c_str());
    GlobalUnlock(hg);

    SetClipboardData(CF_UNICODETEXT, hg);
    CloseClipboard();
    GlobalFree(hg);
  }
}*/
