#include "app.h"

#define WM_TRAYICON (WM_APP + 1)

static HHOOK kbdHook = NULL;
static HHOOK mseHook = NULL;
static HWND g_hwnd = NULL;
static WNDPROC g_OriginalWndProc = NULL;
static AppContext* g_ctx = NULL;

static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* pKeyBoard = (KBDLLHOOKSTRUCT*)lParam;
            if (pKeyBoard->vkCode == 'Q') {
                if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
                    (GetAsyncKeyState(VK_MENU) & 0x8000) &&
                    (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {

                    if (g_ctx != NULL && g_ctx->hwnd != NULL) {
                        StopSound(g_ctx->alarmSound);
                        RestoreWindowClickable(g_ctx->hwnd); 

                        if (g_ctx->currentScheduleList != NULL) {
                            freeLinkedList(g_ctx->currentScheduleList);
                            g_ctx->currentScheduleList = NULL;
                        }

                        g_ctx->currentState = STATE_SETUP;
                        ShowCursor();
                        ShowWindow(g_ctx->hwnd, SW_SHOW);
                        SetForegroundWindow(g_ctx->hwnd);
                        CleanupSystemLocks();
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
    if (nCode >= 0) return 1; 
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void MakeWindowClickThrough(HWND hwnd) {
    if (hwnd == NULL) return;
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_LAYERED);
}

void RestoreWindowClickable(HWND hwnd) {
    if (hwnd == NULL) return;
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
}

static LRESULT CALLBACK CustomWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAYICON) {
        if (lParam == WM_LBUTTONUP) {
            if (g_ctx != NULL && (g_ctx->currentState == STATE_WORKING || g_ctx->currentState == STATE_WARNING)) {
                RestoreWindowClickable(hwnd);
                g_ctx->currentState = STATE_STATUS;
                ShowWindow(hwnd, SW_SHOW);
                SetForegroundWindow(hwnd);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }
        }
        else if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreatePopupMenu();
            InsertMenuA(hMenu, -1, MF_BYPOSITION | MF_STRING, 1001, "Show");
            InsertMenuA(hMenu, -1, MF_BYPOSITION | MF_STRING, 1002, "Close");

            SetForegroundWindow(hwnd);
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);

            if (cmd == 1001 && g_ctx != NULL) {
                if (g_ctx->currentState == STATE_WORKING || g_ctx->currentState == STATE_WARNING) {
                    RestoreWindowClickable(hwnd);
                    g_ctx->currentState = STATE_STATUS;
                    ShowWindow(hwnd, SW_SHOW);
                    SetForegroundWindow(hwnd);
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
            }
            else if (cmd == 1002 && g_ctx != NULL) {
                g_ctx->isRunning = false;
            }
        }
        return 0;
    }
    return CallWindowProc(g_OriginalWndProc, hwnd, msg, wParam, lParam);
}

void InitSystemManager(AppContext* ctx) {
    if (ctx == NULL || ctx->hwnd == NULL) return;
    g_ctx = ctx;
    g_hwnd = ctx->hwnd;
    if (g_OriginalWndProc == NULL) {
        g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(ctx->hwnd, GWLP_WNDPROC, (LONG_PTR)CustomWndProc);
    }
}

void EnableSystemLocks(void) {
    HideCursor();
    kbdHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    mseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
}

void CleanupSystemLocks(void) {
    if (kbdHook != NULL) { UnhookWindowsHookEx(kbdHook); kbdHook = NULL; }
    if (mseHook != NULL) { UnhookWindowsHookEx(mseHook); mseHook = NULL; }
    if (g_hwnd != NULL) { RemoveTrayIcon(g_hwnd); }
}

void AddTrayIcon(HWND hwnd, const char* tooltip) {
    if (hwnd == NULL || tooltip == NULL) return;
    NOTIFYICONDATAA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = 1001;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);

    sprintf_s(nid.szTip, sizeof(nid.szTip), "%s", tooltip);
    Shell_NotifyIconA(NIM_ADD, &nid);
}

void UpdateTrayIconTooltip(HWND hwnd, const char* tooltip) {
    if (hwnd == NULL || tooltip == NULL) return;
    NOTIFYICONDATAA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = 1001;
    nid.uFlags = NIF_TIP;

    sprintf_s(nid.szTip, sizeof(nid.szTip), "%s", tooltip);
    Shell_NotifyIconA(NIM_MODIFY, &nid);
}

void RemoveTrayIcon(HWND hwnd) {
    if (hwnd == NULL) return;
    NOTIFYICONDATAA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATAA);
    nid.hWnd = hwnd;
    nid.uID = 1001;
    Shell_NotifyIconA(NIM_DELETE, &nid);
}

void FetchWindowsUsername(AppContext* ctx) {
    if (ctx == NULL) return;
    DWORD winNameLen = sizeof(ctx->userName);
    GetUserNameA(ctx->userName, &winNameLen);
    ctx->nameLength = (int)strlen(ctx->userName);
}