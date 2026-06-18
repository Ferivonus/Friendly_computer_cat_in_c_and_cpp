#include "app.h"
#include <string.h>

#define WM_TRAYICON       (WM_APP + 1)
#define WM_EMERGENCY_EXIT (WM_APP + 2)

#define TRAY_ICON_ID   1001
#define CMD_TRAY_SHOW  1001
#define CMD_TRAY_CLOSE 1002

static struct {
    HHOOK kbd;
    HHOOK mse;
    HWND hwnd;
    WNDPROC origProc;
    AppContext* ctx;
    HICON icon;
} g_sys = { 0 };

static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= HC_ACTION) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pKb = (KBDLLHOOKSTRUCT*)lParam;
            if (pKb->vkCode == 'Q') {
                if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
                    (GetAsyncKeyState(VK_MENU) & 0x8000) &&
                    (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {

                    if (g_sys.hwnd != NULL) {
                        PostMessage(g_sys.hwnd, WM_EMERGENCY_EXIT, 0, 0);
                    }
                    return 1;
                }
            }
        }
        return 1;
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= HC_ACTION) return 1;
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void MakeWindowClickThrough(HWND hwnd) {
    if (!hwnd) return;
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED);
}

void RestoreWindowClickable(HWND hwnd) {
    if (!hwnd) return;
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
}

static LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_EMERGENCY_EXIT) {
        if (g_sys.ctx != NULL) {
            StopSound(g_sys.ctx->alarmSound);
            RestoreWindowClickable(hwnd);

            if (g_sys.ctx->currentScheduleList != NULL) {
                freeLinkedList(g_sys.ctx->currentScheduleList);
                g_sys.ctx->currentScheduleList = NULL;
            }

            g_sys.ctx->currentState = STATE_SETUP;
            ShowCursor();
            ShowWindow(hwnd, SW_SHOW);
            SetForegroundWindow(hwnd);
            CleanupSystemLocks();
        }
        return 0;
    }

    if (msg == WM_TRAYICON) {
        if (lParam == WM_LBUTTONUP) {
            if (g_sys.ctx != NULL && (g_sys.ctx->currentState == STATE_WORKING || g_sys.ctx->currentState == STATE_WARNING)) {
                RestoreWindowClickable(hwnd);
                g_sys.ctx->currentState = STATE_STATUS;
                ShowWindow(hwnd, SW_SHOW);
                SetForegroundWindow(hwnd);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
        }
        else if (lParam == WM_RBUTTONUP) {
            POINT pt;
            if (GetCursorPos(&pt)) {
                HMENU hMenu = CreatePopupMenu();
                if (hMenu) {
                    InsertMenuA(hMenu, -1, MF_BYPOSITION | MF_STRING, CMD_TRAY_SHOW, "Show");
                    InsertMenuA(hMenu, -1, MF_BYPOSITION | MF_STRING, CMD_TRAY_CLOSE, "Close");

                    SetForegroundWindow(hwnd);
                    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                    DestroyMenu(hMenu);

                    if (cmd == CMD_TRAY_SHOW && g_sys.ctx != NULL) {
                        if (g_sys.ctx->currentState == STATE_WORKING || g_sys.ctx->currentState == STATE_WARNING) {
                            RestoreWindowClickable(hwnd);
                            g_sys.ctx->currentState = STATE_STATUS;
                            ShowWindow(hwnd, SW_SHOW);
                            SetForegroundWindow(hwnd);
                            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        }
                    }
                    else if (cmd == CMD_TRAY_CLOSE && g_sys.ctx != NULL) {
                        g_sys.ctx->isRunning = false;
                    }
                }
            }
        }
        return 0;
    }
    return CallWindowProc(g_sys.origProc, hwnd, msg, wParam, lParam);
}

void InitSystemManager(AppContext* ctx) {
    if (!ctx || !ctx->hwnd) return;
    g_sys.ctx = ctx;
    g_sys.hwnd = ctx->hwnd;
    if (!g_sys.origProc) {
        g_sys.origProc = (WNDPROC)SetWindowLongPtr(g_sys.hwnd, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);
    }
}

void EnableSystemLocks(void) {
    HideCursor();
    HMODULE hMod = GetModuleHandle(NULL);
    if (!g_sys.kbd) g_sys.kbd = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hMod, 0);
    if (!g_sys.mse) g_sys.mse = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, hMod, 0);
}

void CleanupSystemLocks(void) {
    if (g_sys.kbd) { UnhookWindowsHookEx(g_sys.kbd); g_sys.kbd = NULL; }
    if (g_sys.mse) { UnhookWindowsHookEx(g_sys.mse); g_sys.mse = NULL; }
    if (g_sys.hwnd) { RemoveTrayIcon(g_sys.hwnd); }
    if (g_sys.icon) { DestroyIcon(g_sys.icon); g_sys.icon = NULL; }
}

void AddTrayIcon(HWND hwnd, const char* tooltip) {
    if (!hwnd || !tooltip) return;

    if (!g_sys.icon) {
        const char* rawPath = (g_sys.ctx && g_sys.ctx->assetAppIcon[0] != '\0') ? g_sys.ctx->assetAppIcon : DEFAULT_APP_ICON;

        char fixedPath[MAX_PATH];
        strncpy_s(fixedPath, MAX_PATH, rawPath, _TRUNCATE);
        for (int i = 0; fixedPath[i] != '\0'; i++) {
            if (fixedPath[i] == '/') fixedPath[i] = '\\';
        }

        int cx = GetSystemMetrics(SM_CXSMICON);
        int cy = GetSystemMetrics(SM_CYSMICON);

        g_sys.icon = (HICON)LoadImageA(NULL, fixedPath, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);

        if (!g_sys.icon) {
            g_sys.icon = (HICON)LoadImageA(NULL, fixedPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
        }

        if (!g_sys.icon || (UINT_PTR)g_sys.icon <= 1) {
            char exePath[MAX_PATH];
            if (GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
                g_sys.icon = ExtractIconA(NULL, exePath, 0);
            }
        }

        if (!g_sys.icon || (UINT_PTR)g_sys.icon <= 1) {
            g_sys.icon = LoadIcon(NULL, IDI_INFORMATION);
        }
    }

    NOTIFYICONDATAA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = g_sys.icon;

    strncpy_s(nid.szTip, sizeof(nid.szTip), tooltip, _TRUNCATE);
    Shell_NotifyIconA(NIM_ADD, &nid);
}

void UpdateTrayIconTooltip(HWND hwnd, const char* tooltip) {
    if (!hwnd || !tooltip) return;

    NOTIFYICONDATAA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_TIP;

    strncpy_s(nid.szTip, sizeof(nid.szTip), tooltip, _TRUNCATE);
    Shell_NotifyIconA(NIM_MODIFY, &nid);
}

void RemoveTrayIcon(HWND hwnd) {
    if (!hwnd) return;

    NOTIFYICONDATAA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    Shell_NotifyIconA(NIM_DELETE, &nid);
}

void FetchWindowsUsername(AppContext* ctx) {
    if (!ctx) return;

    DWORD winNameLen = sizeof(ctx->userName);
    if (GetUserNameA(ctx->userName, &winNameLen)) {
        ctx->nameLength = (int)winNameLen - 1;
    }
    else {
        ctx->userName[0] = '\0';
        ctx->nameLength = 0;
    }
}