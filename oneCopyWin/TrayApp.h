#ifndef CP_APP_MAC
#define CP_APP_MAC

#include <windows.h>
#include <shellapi.h>
#include <map>
#include <functional>

class TrayApp {
public:
  int run(HINSTANCE instance);

  static TrayApp& getInstance();
  HWND createWindow();
  HMENU createContext();

private:
  //singleton
  TrayApp();
  ~TrayApp();
  TrayApp(TrayApp const&);
  void operator=(TrayApp const&) = delete;

  typedef std::function<void()> buttonCallback;

  HINSTANCE instance_;
  HWND window_;
  HICON iconDEFAULT_;
  NOTIFYICONDATA notificationData_;

  std::string tooltip_;
  bool addedIcon_;

  //notification area icon & menu
  HMENU contextMenu_;
  void addMenuItem(std::string title, buttonCallback callback);
  void addMenuSpacer();
  std::map<int, buttonCallback> callbacks_;

  void update(std::string title = "", std::string message = "");

public:
  static LRESULT CALLBACK windowCB(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  /*
  notification area icon stuff
  */
  static void removeIcon();
  static void showNotification(std::string title, std::string message);
  static void setTooltip(std::string tooltip);



  /*
  button actions
  */
  static void pushAction();
  static void pullAction();
  static void settingsAction();
  static void aboutAction();
  static void quitAction();
};


#endif