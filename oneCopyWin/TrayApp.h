#ifndef CP_APP_MAC
#define CP_APP_MAC

#include <windows.h>
#include <shellapi.h>
#include <map>
#include <functional>

namespace cplan {

  class TrayApp {
  public:
    int run();

    HANDLE thread_;
    static TrayApp& getInstance();
    HWND createWindow(HINSTANCE inst);
    HMENU createContext();
  private:
    //singleton
    TrayApp();
    ~TrayApp();
    TrayApp(TrayApp const&);
    void operator=(TrayApp const&) = delete;
    void doQuit();

    typedef std::function<void()> buttonCallback;

    HWND window_;
    HICON iconDEFAULT_;
    NOTIFYICONDATA notificationData_;

    std::string tooltip_;

    bool addedIcon_;

    //context menu
    HMENU contextMenu_;
    void addMenuItem(std::string title, buttonCallback callback);
    void addMenuSpacer();
    std::map<int, buttonCallback> callbacks_;

    void update(std::string title = "", std::string message = "");

    static void toClipboard(const std::wstring &wstr);
    static std::string fromClipboard();
  public:

    static LRESULT CALLBACK windowCB(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    static void removeIcon();
    void exit();
    /*
    show notification & icon
    */
    static void showNotification(std::string title, std::string message);

    /*
    windows only
    there are no tooltips in macOS
    */
    static void setTooltip(std::string tooltip);

    static void pushAction();

    static void pullAction();

    static void settingsAction();
  };
}

#endif