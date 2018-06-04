
#include "stdafx.h"
#include "TrayApp.h"

#include <Shlwapi.h>
#include <psapi.h>

#include <sstream>
#include <memory>

#include "resource.h"

#include "PocoRequest.h"
#include "B64.h"
#include "Config.h"
#include "Clipboard.h"
#include "Util.h"

#ifdef UNICODE
#define stringcopy wcscpy_s
#else
#define stringcopy strcpy
#endif

#define ID_TRAY_APP_ICON                355633
#define WM_TRAYICON ( WM_USER + 1 )
#define TRAY_QUIT_MSG ( WM_USER + 2 )



int TrayApp::run(HINSTANCE instance) {
  instance_ = instance;

  createWindow();

  TrayApp::update("started app!", "oneCopy");

  //run main loop
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


TrayApp::TrayApp() :
  window_(nullptr),
  iconDEFAULT_(nullptr),
  addedIcon_(false),
  contextMenu_(nullptr)
{

}

TrayApp::~TrayApp() {
  DestroyMenu(contextMenu_);
  DestroyWindow(window_);
  removeIcon();
}

TrayApp & TrayApp::getInstance() {
  static TrayApp app;
  return app;
}

HWND TrayApp::createWindow() {
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

  //create notification area icon 
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

HMENU TrayApp::createContext() {
  //initialize menu
  contextMenu_ = CreatePopupMenu();
  addMenuItem("push", &TrayApp::pushAction);
  addMenuItem("pull", &TrayApp::pullAction);
  addMenuSpacer();
  addMenuItem("settings", &TrayApp::settingsAction);
  //addMenuItem("about", &TrayApp::aboutAction);
  addMenuItem("exit", &TrayApp::quitAction);
  return contextMenu_;
}

// Message handler for about box.
INT_PTR CALLBACK About2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG: {
    std::wstring apikey = Util::toWStr(Config::getAPIKeyOne());
    SetDlgItemText(hDlg, IDC_EDIT1, apikey.c_str());

    std::wstring addr = Util::toWStr(Config::getServerAddr());
    SetDlgItemText(hDlg, IDC_EDIT2, addr.c_str());

    return (INT_PTR)TRUE;
  }

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK)
    {
      EndDialog(hDlg, LOWORD(wParam));

      WCHAR apikey[256];
      GetDlgItemText(hDlg, IDC_EDIT1, apikey, 256);
      Config::setAPIKey(Util::toStr(apikey));

      WCHAR server[256];
      GetDlgItemText(hDlg, IDC_EDIT2, server, 256);
      Config::setServerAddr(Util::toStr(server));

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


LRESULT CALLBACK TrayApp::windowCB(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
  //wParam: try icon id

  switch (message)
  {
    /*case WM_COMMAND:
    {
      int wmId = LOWORD(wParam);
      // Parse the menu selections:
      switch (wmId)
      {
      case IDM_ABOUT:
        DialogBox(TrayApp::getInstance().instance_, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About2);
        break;
      case IDM_SETTINGS:
        DialogBox(TrayApp::getInstance().instance_, MAKEINTRESOURCE(IDD_SETTINGS), hwnd, About2);
        break;
      default:
        return DefWindowProc(hwnd, message, wParam, lParam);
      }
    }
    break;*/
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

    default:
      break;
  }

  return DefWindowProc(hwnd, message, wParam, lParam);
}

void TrayApp::removeIcon() {
  Shell_NotifyIcon(NIM_DELETE, &(getInstance().notificationData_));
}


void TrayApp::addMenuItem(std::string title, buttonCallback callback) {
  if (!contextMenu_) {
    return;
  }

  static int callbackID = 0;
  callbackID++;

  callbacks_.emplace(callbackID, callback);

  AppendMenuA(contextMenu_, MF_STRING, callbackID, title.c_str());
}

void TrayApp::addMenuSpacer() {
  if (!contextMenu_) {
    return;
  }

  AppendMenu(contextMenu_, MF_MENUBREAK, 0, nullptr);
}


void TrayApp::update(std::string title, std::string message) {

  // receive update from pipe , but window is not yet created, or other
  if (NULL == window_)
  {
    return;
  }

  if (!title.empty() && !message.empty()) {
    //set message  & title
    std::wstring titleW(title.length(), L' ');
    std::copy(title.begin(), title.end(), titleW.begin());
    stringcopy(notificationData_.szInfo, titleW.c_str());

    std::wstring messageW(message.length(), L' ');
    std::copy(message.begin(), message.end(), messageW.begin());
    stringcopy(notificationData_.szInfoTitle, messageW.c_str());
  }

  if (!tooltip_.empty()) {
    //set tooltip
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

void TrayApp::showNotification(std::string title, std::string message) {
  getInstance().update(title, message);
}

void TrayApp::setTooltip(std::string tooltip) {
  getInstance().tooltip_ = tooltip;
  getInstance().update();
}

void TrayApp::pushAction() {
  auto value = Clipboard::getString();
  auto apiKey = Config::getAPIKeyOne();

  //copied text -> direct upload
  if (!value.empty()) {
    auto valueEncoded = B64::encode(value.c_str(), value.length());
    std::stringstream json;
    json << "{\"type\":\"set_key\",\"key\":\"" << apiKey << "\",\"value\":\"" << valueEncoded << "\"}";

    PocoRequest request(Config::getServerAddr());
    auto reply = request.post(json.str());

    return;
  }
  
  auto filePath = Clipboard::getFilePath();
  if (!filePath.empty()) {
    //upload file name as key
    auto fileName = Util::toStr(Util::fileNameFromPath(filePath));

    auto valueEncoded = B64::encode(fileName.c_str(), fileName.length());
    std::stringstream json;
    json << "{\"type\":\"set_key\",\"key\":\"" << apiKey << "\",\"value\":\"" << valueEncoded << "\"}";

    PocoRequest request(Config::getServerAddr());
    auto valueReply = request.post(json.str());

    
    //upload file
    auto fileSize = Util::getFileSize(filePath);
    if (fileSize > 1000000) {
      return;
    }

    std::unique_ptr<char[]> data(new char[fileSize]);
    Util::readFile(filePath, data.get(), fileSize);

    std::stringstream uploadURL;
    uploadURL << Config::getServerAddr() << "/file/" << apiKey;
    PocoRequest uploadRequest(uploadURL.str());
    auto fileReply = uploadRequest.post(data.get(), fileSize);

    
    //build link & copy link to clipboard
    std::stringstream fileLink;
    fileLink << Config::getServerAddr() << "/file/" << apiKey << "/" << fileName;

    Clipboard::setString(fileLink.str());

  }
}

void TrayApp::pullAction() {
  auto apiKey = Config::getAPIKeyOne();
  
  std::stringstream json;
  json << "{\"type\":\"get_key\",\"key\":\"" << apiKey << "\"}";

  PocoRequest request(Config::getServerAddr());
  auto reply = request.post(json.str());

  auto val = request.getString("value");
  
  //decode base64
  std::unique_ptr<char[]> data(new char[B64::b64DecodeSize(val.length())]);
  auto dataSize = B64::decode(val, *data.get(), B64::b64DecodeSize(val.length())) - 1;
  if (dataSize < 0) {
    return;
  }
  std::string valStr(data.get(), dataSize);

  //TODO: check if it is a file
  //TODO: download file
  //TODO: set file to clipboard

  Clipboard::setString(valStr);
}

void TrayApp::settingsAction() {
  TrayApp::showNotification("settings!", "oneCopy");

  DialogBox(TrayApp::getInstance().instance_, MAKEINTRESOURCE(IDD_SETTINGS), TrayApp::getInstance().window_, About2);
}

void TrayApp::aboutAction() {
  DialogBox(TrayApp::getInstance().instance_, MAKEINTRESOURCE(IDD_ABOUTBOX), TrayApp::getInstance().window_, About2);
}

void TrayApp::quitAction() {
  PostQuitMessage(0);
}
