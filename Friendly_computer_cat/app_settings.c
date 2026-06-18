#include "app.h"

void SaveSettings(AppContext* ctx) {
    if (ctx == NULL) return;

    CreateDirectoryA(CONFIG_DIR, NULL);
    char buffer[32];

    sprintf_s(buffer, sizeof(buffer), "%d", ctx->workMin);
    WritePrivateProfileStringA("Durations", "WorkMin", buffer, INI_FILE_PATH);
    sprintf_s(buffer, sizeof(buffer), "%d", ctx->workSec);
    WritePrivateProfileStringA("Durations", "WorkSec", buffer, INI_FILE_PATH);

    sprintf_s(buffer, sizeof(buffer), "%d", ctx->breakMin);
    WritePrivateProfileStringA("Durations", "BreakMin", buffer, INI_FILE_PATH);
    sprintf_s(buffer, sizeof(buffer), "%d", ctx->breakSec);
    WritePrivateProfileStringA("Durations", "BreakSec", buffer, INI_FILE_PATH);

    sprintf_s(buffer, sizeof(buffer), "%d", ctx->totalCycles);
    WritePrivateProfileStringA("Durations", "TotalCycles", buffer, INI_FILE_PATH);

    sprintf_s(buffer, sizeof(buffer), "%d", ctx->volumeLevel);
    WritePrivateProfileStringA("Durations", "Volume", buffer, INI_FILE_PATH);

    sprintf_s(buffer, sizeof(buffer), "%d", ctx->warningSec);
    WritePrivateProfileStringA("Durations", "WarningSec", buffer, INI_FILE_PATH);

    sprintf_s(buffer, sizeof(buffer), "%d", ctx->panelW);
    WritePrivateProfileStringA("Window", "PanelW", buffer, INI_FILE_PATH);
    sprintf_s(buffer, sizeof(buffer), "%d", ctx->panelH);
    WritePrivateProfileStringA("Window", "PanelH", buffer, INI_FILE_PATH);

    WritePrivateProfileStringA("Assets", "WorkGif", ctx->assetWorkGif, INI_FILE_PATH);
    WritePrivateProfileStringA("Assets", "BreakGif", ctx->assetBreakGif, INI_FILE_PATH);
    WritePrivateProfileStringA("Assets", "InfoGif", ctx->assetInfoGif, INI_FILE_PATH);
    WritePrivateProfileStringA("Assets", "GoodbyeGif", ctx->assetGoodbyeGif, INI_FILE_PATH);
    WritePrivateProfileStringA("Assets", "AlarmSound", ctx->assetAlarmSound, INI_FILE_PATH);
    WritePrivateProfileStringA("Assets", "AppIcon", ctx->assetAppIcon, INI_FILE_PATH);

    WritePrivateProfileStringA("Preferences", "ReminderMessage", ctx->reminderMessage, INI_FILE_PATH);

    for (int i = 0; i < QUOTE_COUNT; i++) {
        char keyBuf[16];
        sprintf_s(keyBuf, sizeof(keyBuf), "Quote%d", i + 1);
        WritePrivateProfileStringA("Quotes", keyBuf, ctx->motivationalQuotes[i], INI_FILE_PATH);
    }
}

void LoadSettings(AppContext* ctx) {
    if (ctx == NULL) return;

    CreateDirectoryA(CONFIG_DIR, NULL);

    const char* defaultQuotes[QUOTE_COUNT] = {
        "Time is your most valuable asset.",
        "Where focus goes, energy flows.",
        "Deep work is rare and valuable.",
        "Master your attention, master your life.",
        "Small disciplines lead to huge results."
    };
    const char* defaultReminder = "Time to focus on your task!";

    DWORD attrib = GetFileAttributesA(INI_FILE_PATH);
    if (attrib == INVALID_FILE_ATTRIBUTES) {
        ctx->workMin = DEFAULT_WORK_MIN;
        ctx->workSec = DEFAULT_WORK_SEC;
        ctx->breakMin = DEFAULT_BREAK_MIN;
        ctx->breakSec = DEFAULT_BREAK_SEC;
        ctx->totalCycles = DEFAULT_CYCLES;
        ctx->volumeLevel = DEFAULT_VOLUME;
        ctx->warningSec = 10;

        ctx->panelW = DEFAULT_PANEL_W;
        ctx->panelH = DEFAULT_PANEL_H;

        strcpy_s(ctx->assetWorkGif, sizeof(ctx->assetWorkGif), DEFAULT_WORK_GIF_PATH);
        strcpy_s(ctx->assetBreakGif, sizeof(ctx->assetBreakGif), DEFAULT_BREAK_GIF_PATH);
        strcpy_s(ctx->assetInfoGif, sizeof(ctx->assetInfoGif), DEFAULT_INFO_GIF_PATH);
        strcpy_s(ctx->assetGoodbyeGif, sizeof(ctx->assetGoodbyeGif), DEFAULT_GOODBYE_GIF_PATH);
        strcpy_s(ctx->assetAlarmSound, sizeof(ctx->assetAlarmSound), DEFAULT_ALARM_SOUND);
        strcpy_s(ctx->assetAppIcon, sizeof(ctx->assetAppIcon), DEFAULT_APP_ICON);

        strcpy_s(ctx->reminderMessage, sizeof(ctx->reminderMessage), defaultReminder);

        for (int i = 0; i < QUOTE_COUNT; i++) {
            strcpy_s(ctx->motivationalQuotes[i], sizeof(ctx->motivationalQuotes[i]), defaultQuotes[i]);
        }
        SaveSettings(ctx);
        return;
    }

    ctx->workMin = GetPrivateProfileIntA("Durations", "WorkMin", DEFAULT_WORK_MIN, INI_FILE_PATH);
    ctx->workSec = GetPrivateProfileIntA("Durations", "WorkSec", DEFAULT_WORK_SEC, INI_FILE_PATH);
    ctx->breakMin = GetPrivateProfileIntA("Durations", "BreakMin", DEFAULT_BREAK_MIN, INI_FILE_PATH);
    ctx->breakSec = GetPrivateProfileIntA("Durations", "BreakSec", DEFAULT_BREAK_SEC, INI_FILE_PATH);
    ctx->totalCycles = GetPrivateProfileIntA("Durations", "TotalCycles", DEFAULT_CYCLES, INI_FILE_PATH);
    ctx->volumeLevel = GetPrivateProfileIntA("Durations", "Volume", DEFAULT_VOLUME, INI_FILE_PATH);
    ctx->warningSec = GetPrivateProfileIntA("Durations", "WarningSec", 10, INI_FILE_PATH);

    ctx->panelW = GetPrivateProfileIntA("Window", "PanelW", DEFAULT_PANEL_W, INI_FILE_PATH);
    ctx->panelH = GetPrivateProfileIntA("Window", "PanelH", DEFAULT_PANEL_H, INI_FILE_PATH);

    GetPrivateProfileStringA("Assets", "WorkGif", DEFAULT_WORK_GIF_PATH, ctx->assetWorkGif, MAX_PATH, INI_FILE_PATH);
    GetPrivateProfileStringA("Assets", "BreakGif", DEFAULT_BREAK_GIF_PATH, ctx->assetBreakGif, MAX_PATH, INI_FILE_PATH);
    GetPrivateProfileStringA("Assets", "InfoGif", DEFAULT_INFO_GIF_PATH, ctx->assetInfoGif, MAX_PATH, INI_FILE_PATH);
    GetPrivateProfileStringA("Assets", "GoodbyeGif", DEFAULT_GOODBYE_GIF_PATH, ctx->assetGoodbyeGif, MAX_PATH, INI_FILE_PATH);
    GetPrivateProfileStringA("Assets", "AlarmSound", DEFAULT_ALARM_SOUND, ctx->assetAlarmSound, MAX_PATH, INI_FILE_PATH);
    GetPrivateProfileStringA("Assets", "AppIcon", DEFAULT_APP_ICON, ctx->assetAppIcon, MAX_PATH, INI_FILE_PATH);

    GetPrivateProfileStringA("Preferences", "ReminderMessage", defaultReminder, ctx->reminderMessage, 256, INI_FILE_PATH);

    for (int i = 0; i < QUOTE_COUNT; i++) {
        char keyBuf[16];
        sprintf_s(keyBuf, sizeof(keyBuf), "Quote%d", i + 1);
        GetPrivateProfileStringA("Quotes", keyBuf, defaultQuotes[i], ctx->motivationalQuotes[i], 256, INI_FILE_PATH);
    }
}