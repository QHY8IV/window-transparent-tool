#include <windows.h>
#include <cstdio>
#include <unordered_set>

bool IsProcessRunAsAdmin() {
  SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  PSID AdministratorsGroup;
  BOOL b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
  if (b) {
    CheckTokenMembership(NULL, AdministratorsGroup, &b);
    FreeSid(AdministratorsGroup);
  } return b == TRUE;
}


int ManagerRun() {
  if (IsProcessRunAsAdmin()) return 2;
  SHELLEXECUTEINFO ShExecInfo;
#ifdef UNICODE
  static WCHAR path[1024];
  memset(path, 0X00, sizeof path);
  GetModuleFileName(NULL, path, 1024);
  ShExecInfo.lpVerb = L"runas";
  ShExecInfo.lpFile = path;
#else
  static CHAR path[1024];
  memset(path, 0X00, sizeof path);
  GetModuleFileName(NULL, path, 1024);
  ShExecInfo.lpVerb = "runas";
  ShExecInfo.lpFile = path;
#endif
  ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
  ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
  ShExecInfo.hwnd = NULL;
  ShExecInfo.lpParameters = NULL;
  ShExecInfo.lpDirectory = NULL;
  ShExecInfo.nShow = SW_SHOWDEFAULT;
  ShExecInfo.hInstApp = NULL;
  return ShellExecuteEx(&ShExecInfo)? 1: 0;
}

enum {
  INC_ALPHA,
  DEC_ALPHA,
  SET_STEP,
  ERASE_ALPHA,
  SET_TOP,
  MV_LEFT,
  MV_RIGHT,
  MV_UP,
  MV_DOWN,
  MK_LEFT,
  MK_RIGHT,
  MK_UP,
  MK_DOWN,
  MV_SET_STEP,
  MK_SET_STEP,
  HELP,
  QUIT
};

int ALPHA_STEP = 10, MOVE_STEP = 10, SIZE_STEP = 10;

template<class... T>
void alert(const char *fmt, T... args) {
  static char buf[1024];
  sprintf(buf, fmt, args...);
  MessageBox(NULL, buf, NULL, MB_ICONWARNING);
}

template<class... T>
void showinfo(const char *fmt, T... args) {
  static char buf[1024];
  sprintf(buf, fmt, args...);
  MessageBox(NULL, buf, "", MB_ICONINFORMATION);
}

void error(int code) {
  alert("程序异常退出，错误代码：%d\n", code);
  exit(code);
}

void help() {
  showinfo(
    "Alt+Left/Right\t\t降低/增加透明度\n"
    "Alt+H\t\t\t取消透明度\n"
    "Alt+S\t\t\t设置透明度增量\n"
    "Ctrl+Alt+Left/Right/Up/Down\t移动窗口\n"
    "Ctrl+Alt+S\t\t\t设置窗口移动步长\n"
    "Shift+Alt+Left/Right/Up/Down\t缩放窗口\n"
    "Shift+Alt+S\t\t设置窗口缩放步长\n"
    "Alt+T\t\t\t置顶/取消置顶窗口\n"
    "Alt+E\t\t\t显示此帮助\n"
    "Alt+ESC\t\t\t退出\n");
}

void helpfirst() {
  showinfo("欢迎使用 C++ 小工具！\n"
    "Alt+Left/Right\t\t降低/增加透明度\n"
    "Alt+H\t\t\t取消透明度\n"
    "Alt+S\t\t\t设置透明度增量\n"
    "Ctrl+Alt+Left/Right/Up/Down\t移动窗口\n"
    "Ctrl+Alt+S\t\t\t设置窗口移动步长\n"
    "Shift+Alt+Left/Right/Up/Down\t缩放窗口\n"
    "Shift+Alt+S\t\t设置窗口缩放步长\n"
    "Alt+T\t\t\t置顶/取消置顶窗口\n"
    "Alt+E\t\t\t显示此帮助\n"
    "Alt+ESC\t\t\t退出\n");
}

int handleHotkey(int hotkey) {

  using uint = unsigned int;

  HWND hActiveWindow;
  BYTE alpha;
  RECT rect;

  if (hotkey == INC_ALPHA || hotkey == DEC_ALPHA || hotkey == ERASE_ALPHA || hotkey == SET_TOP
    || hotkey == MV_LEFT || hotkey == MV_RIGHT || hotkey == MV_UP || hotkey == MV_DOWN
    || hotkey == MK_LEFT || hotkey == MK_RIGHT || hotkey == MK_UP || hotkey == MK_DOWN) {
    if ((hActiveWindow = GetForegroundWindow()) == NULL) {
      alert("GetForegroundWindow 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    }
  }

  if (hotkey == INC_ALPHA || hotkey == DEC_ALPHA) {
    if (!SetWindowLong(hActiveWindow, GWL_EXSTYLE, GetWindowLong(hActiveWindow, GWL_EXSTYLE) | WS_EX_LAYERED)) {
      alert("SetWindowLong 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    } DWORD flag;
    if (!GetLayeredWindowAttributes(hActiveWindow, NULL, &alpha, &flag)) {
      alert("GetLayeredWindowAttributes 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    } if (!(flag & LWA_ALPHA)) {
      SetLayeredWindowAttributes(hActiveWindow, 0, alpha = 255, LWA_ALPHA);
    }
  }

  if (hotkey == MV_LEFT || hotkey == MV_RIGHT || hotkey == MV_UP || hotkey == MV_DOWN
    || hotkey == MK_LEFT || hotkey == MK_RIGHT || hotkey == MK_UP || hotkey == MK_DOWN) {
    if (!GetWindowRect(hActiveWindow, &rect)) {
      alert("GetWindowRect 调用失败！错误代码：%d\n", int(GetLastError()));
    }
  }

  if (hotkey == INC_ALPHA) {
    if (alpha >= 255 - ALPHA_STEP) alpha = 255;
    else alpha = alpha + ALPHA_STEP;
    if (!SetLayeredWindowAttributes(hActiveWindow, 0, alpha, LWA_ALPHA)) {
      alert("SetLayeredWindowAttributes 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    } return 0;
  }

  if (hotkey == DEC_ALPHA) {
    if (alpha <= ALPHA_STEP) alpha = 0;
    else alpha = alpha - ALPHA_STEP;
    if (!SetLayeredWindowAttributes(hActiveWindow, 0, alpha, LWA_ALPHA)) {
      alert("SetLayeredWindowAttributes 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    } return 0;
  }

  if (hotkey == SET_STEP) {
    showinfo("功能尚未开发，敬请期待。");
    return 0;
  }

  if (hotkey == ERASE_ALPHA) {
    if (!SetWindowLong(hActiveWindow, GWL_EXSTYLE, GetWindowLong(hActiveWindow, GWL_EXSTYLE) & ~WS_EX_LAYERED)) {
      alert("SetWindowLong 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    } if (!RedrawWindow(hActiveWindow, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN)) {
      alert("RedrawWindow 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    }
  }

  if (hotkey == SET_TOP) {
    WINDOWINFO info;
    memset(&info, 0, sizeof(info));
    info.cbSize = sizeof(WINDOWINFO);
    if (!GetWindowInfo(hActiveWindow, &info)) {
      alert("GetWindowInfo 调用失败！错误代码：%d\n", int(GetLastError()));
      return 0;
    } if (info.dwExStyle & WS_EX_TOPMOST) {
      SetWindowPos(hActiveWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    } else {
      SetWindowPos(hActiveWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
  }

  if (hotkey == MV_LEFT) {
    SetWindowPos(hActiveWindow, NULL, rect.left - MOVE_STEP, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MV_UP) {
    SetWindowPos(hActiveWindow, NULL, rect.left, rect.top - MOVE_STEP, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MV_RIGHT) {
    SetWindowPos(hActiveWindow, NULL, rect.left + MOVE_STEP, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MV_DOWN) {
    SetWindowPos(hActiveWindow, NULL, rect.left, rect.top + MOVE_STEP, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MK_LEFT) {
    SetWindowPos(hActiveWindow, NULL, 0, 0, rect.right - rect.left - SIZE_STEP, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MK_UP) {
    SetWindowPos(hActiveWindow, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top - SIZE_STEP, SWP_NOMOVE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MK_RIGHT) {
    SetWindowPos(hActiveWindow, NULL, 0, 0, rect.right - rect.left + SIZE_STEP, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MK_DOWN) {
    SetWindowPos(hActiveWindow, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top + SIZE_STEP, SWP_NOMOVE | SWP_NOZORDER);
    return 0;
  }

  if (hotkey == MV_SET_STEP) {
    showinfo("功能尚未开发，敬请期待。");
    return 0;
  }

  if (hotkey == MK_SET_STEP) {
    showinfo("功能尚未开发，敬请期待。");
    return 0;
  }

  if (hotkey == HELP) {
    help(); return 0;
  }

  if (hotkey == QUIT) {
    showinfo("C++ 小工具已退出，感谢你的使用。\n");
    return 1;
  }

  return 0;
}

int main(int argc, char** argv) {

  switch (ManagerRun()) {
    case 0: alert("请以管理员身份运行！\n");
    case 1: return 0;
    default: break;
  }

  helpfirst();

  if (!RegisterHotKey(NULL, INC_ALPHA, MOD_ALT, VK_RIGHT)
    || !RegisterHotKey(NULL, DEC_ALPHA, MOD_ALT, VK_LEFT)
    || !RegisterHotKey(NULL, SET_STEP, MOD_ALT, 'S')
    || !RegisterHotKey(NULL, ERASE_ALPHA, MOD_ALT, 'H')
    || !RegisterHotKey(NULL, SET_TOP, MOD_ALT, 'T')
    || !RegisterHotKey(NULL, MV_LEFT, MOD_CONTROL | MOD_ALT, VK_LEFT)
    || !RegisterHotKey(NULL, MV_RIGHT, MOD_CONTROL | MOD_ALT, VK_RIGHT)
    || !RegisterHotKey(NULL, MV_UP, MOD_CONTROL | MOD_ALT, VK_UP)
    || !RegisterHotKey(NULL, MV_DOWN, MOD_CONTROL | MOD_ALT, VK_DOWN)
    || !RegisterHotKey(NULL, MK_LEFT, MOD_SHIFT | MOD_ALT, VK_LEFT)
    || !RegisterHotKey(NULL, MK_RIGHT, MOD_SHIFT | MOD_ALT, VK_RIGHT)
    || !RegisterHotKey(NULL, MK_UP, MOD_SHIFT | MOD_ALT, VK_UP)
    || !RegisterHotKey(NULL, MK_DOWN, MOD_SHIFT | MOD_ALT, VK_DOWN)
    || !RegisterHotKey(NULL, MV_SET_STEP, MOD_CONTROL | MOD_ALT, 'S')
    || !RegisterHotKey(NULL, MK_SET_STEP, MOD_SHIFT | MOD_ALT, 'S')
    || !RegisterHotKey(NULL, HELP, MOD_ALT, 'E')
    || !RegisterHotKey(NULL, QUIT, MOD_ALT, VK_ESCAPE)) {
      error(GetLastError());
    }

  MSG msg; int ret;
  while (ret = GetMessage(&msg, NULL, 0, 0)) {
    if (ret == -1) error(GetLastError());
    if (msg.message == WM_HOTKEY) {
      int message = msg.wParam;
      if (handleHotkey(message)) {
        break;
      }
    }
  }

  if (!UnregisterHotKey(NULL, INC_ALPHA)
    || !UnregisterHotKey(NULL, DEC_ALPHA)
    || !UnregisterHotKey(NULL, SET_STEP)
    || !UnregisterHotKey(NULL, ERASE_ALPHA)
    || !UnregisterHotKey(NULL, SET_TOP)
    || !UnregisterHotKey(NULL, MV_LEFT)
    || !UnregisterHotKey(NULL, MV_RIGHT)
    || !UnregisterHotKey(NULL, MV_UP)
    || !UnregisterHotKey(NULL, MV_DOWN)
    || !UnregisterHotKey(NULL, MK_LEFT)
    || !UnregisterHotKey(NULL, MK_RIGHT)
    || !UnregisterHotKey(NULL, MK_UP)
    || !UnregisterHotKey(NULL, MK_DOWN)
    || !UnregisterHotKey(NULL, MV_SET_STEP)
    || !UnregisterHotKey(NULL, MK_SET_STEP)
    || !UnregisterHotKey(NULL, HELP)
    || !UnregisterHotKey(NULL, QUIT)) {
      error(GetLastError());
    }

  return 0;
}
