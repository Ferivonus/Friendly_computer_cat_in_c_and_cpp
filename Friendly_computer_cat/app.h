#ifndef APP_H
#define APP_H

#include <stdbool.h>

// Resolve naming conflicts between Windows API and Raylib before including them
#define CloseWindow Win32CloseWindow
#define ShowCursor Win32ShowCursor
#define Rectangle Win32Rectangle
#define DrawText Win32DrawText
#define DrawTextEx Win32DrawTextEx
#define LoadImage Win32LoadImage
#define PlaySound Win32PlaySound

#define NOGDI
#define NOIMAGE
#include <windows.h>

#undef CloseWindow
#undef ShowCursor
#undef Rectangle
#undef DrawText
#undef DrawTextEx
#undef LoadImage
#undef PlaySound

#include "raylib.h"
#include "raymath.h" 
#include "app_config.h"

#include "WorkSchedule_Wrapper.h"
#include "LinkedList.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <shellapi.h>

typedef enum {
    STATE_LOADING,
    STATE_SETUP,
    STATE_PROMPT_DB,
    STATE_SCHEDULE_DB,
    STATE_SCHEDULE_DETAIL,
    STATE_WORKING,
    STATE_WARNING,
    STATE_STATUS,
    STATE_LOCKED,
    STATE_BREAK_OVER,
    STATE_FINISHED
} AppState;

typedef struct {
    bool clicked;
    bool howManyTimes;
} add_button_Clicked_struct;

typedef struct {
    AppState currentState;
    bool isRunning;
    HWND hwnd;

    int screenWidth;
    int screenHeight;

    int panelW;
    int panelH;
    bool isResizing;

    float targetPanelW;
    float targetPanelH;

    WorkScheduleHandle dbHandle;
    int scheduleScrollY;
    int selectedScheduleId;
    LinkedList* currentScheduleList; 
    C_WorkScheduleItem* viewedSchedule;

    char promptTitle[64];
    char promptInfo[256];
    int promptActiveField;

    int loadStep;

    char userName[64];
    int nameLength;
    int workMin, workSec;
    int breakMin, breakSec;
    int totalCycles, currentCycle;
    int volumeLevel;
    int warningSec;
    int activeField;

    bool showHelpScreen;

    char assetWorkGif[MAX_PATH];
    char assetBreakGif[MAX_PATH];
    char assetAlarmSound[MAX_PATH];
    char assetInfoGif[MAX_PATH];
    char assetGoodbyeGif[MAX_PATH];

    char reminderMessage[256];
    char motivationalQuotes[QUOTE_COUNT][256];

    float workTimer;
    float lockTimer;
    float breakOverTimer;
    float finishedTimer;
    float frameDelay;

    Sound alarmSound;
    Image imAnim1, imAnim2, imAnim3, imAnim4;
    Texture2D texAnim1, texAnim2, texAnim3, texAnim4;

    int animFrames1, animFrames2, animFrames3, animFrames4;
    int currentAnimFrame1, currentAnimFrame2, currentAnimFrame3, currentAnimFrame4;
    float frameTimer1, frameTimer2, frameTimer3, frameTimer4;

    Color bgDark, bgPanel, textSoft, accentGreen, accentGold, btnNormal, btnHover, activeBox;

    add_button_Clicked_struct add_button_Clicked;
} AppContext;

// System operations (app_sys.c)
void FetchWindowsUsername(AppContext* ctx);
void InitSystemManager(AppContext* ctx);
void EnableSystemLocks(void);
void CleanupSystemLocks(void);
void AddTrayIcon(HWND hwnd, const char* tooltip);
void UpdateTrayIconTooltip(HWND hwnd, const char* tooltip);
void RemoveTrayIcon(HWND hwnd);
void MakeWindowClickThrough(HWND hwnd);
void RestoreWindowClickable(HWND hwnd);

// Settings and asset management (app_settings.c / app_assets.c)
void LoadSettings(AppContext* ctx);
void SaveSettings(AppContext* ctx);
bool UpdateAssetLoading(AppContext* ctx);
void UnloadAppAssets(AppContext* ctx);

// Background logic handlers (app_state.c)
// Note: 'dt' represents delta time to ensure smooth, frame-independent timing.
void UpdateLoadingState(AppContext* ctx, float dt);
void UpdateSetupState(AppContext* ctx);
void UpdateScheduleState(AppContext* ctx); // YENÝ: Veritabaný ekraný mantýk güncelleyicisi
void UpdateWorkingState(AppContext* ctx, float dt);
void UpdateWarningState(AppContext* ctx, float dt);
void UpdateStatusState(AppContext* ctx, float dt);
void UpdateLockedState(AppContext* ctx, float dt);
void UpdateBreakOverState(AppContext* ctx, float dt);
void UpdateFinishedState(AppContext* ctx, float dt);

// Visual rendering handlers (app_draw.c)
void DrawLoadingState(AppContext* ctx);
void DrawSetupState(AppContext* ctx);
void DrawScheduleState(AppContext* ctx); // YENÝ: Veritabaný ekraný çizicisi
void DrawWarningState(AppContext* ctx);
void DrawStatusState(AppContext* ctx);
void DrawLockedState(AppContext* ctx);
void DrawBreakOverState(AppContext* ctx);
void DrawFinishedState(AppContext* ctx);
void UpdateScheduleDetailState(AppContext* ctx);
void DrawScheduleDetailState(AppContext* ctx);

// Referans (&) iþaretinden arýndýrýlmýþ ve birleþtirilmiþ geliþmiþ buton fonksiyonlarý
bool DrawButton(Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color textColor);
bool DrawButtonAdvanced(Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color textColor, bool isEnabled);


// Database Ýþlemleri Ýçin Köprü (Bridge) Fonksiyonlarý (app_db_logic.c)
void HandleAddSchedule(AppContext* ctx);
void HandleDeleteSchedule(AppContext* ctx);
void RefreshScheduleList(AppContext* ctx);

void UpdatePromptDbState(AppContext* ctx);
void DrawPromptDbState(AppContext* ctx);
void HandleUpdateSchedule(AppContext* ctx);

#endif // APP_H