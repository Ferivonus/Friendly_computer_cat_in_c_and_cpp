#include "app.h"
#include <stdio.h>
#include "WorkSchedule_Wrapper.h"
#include <pthread.h>

pthread_mutex_t db_lock;

#ifdef _WIN32
    #ifndef _DEBUG
        #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
    #endif
#endif

// tatlý iþ parçacýðý
static void* thread_work_schedule(void* arg) {
    AppContext* ctx = (AppContext*)arg;

    pthread_mutex_lock(&db_lock);
    WorkScheduleHandle my_db = start_work_schedule_wrapper("work_schedule");

    ctx->dbHandle = my_db;
    pthread_mutex_unlock(&db_lock);

    return NULL;
}

int main(void) {
    AppContext ctx = { 0 };
    ctx.isRunning = true;
    ctx.currentState = STATE_LOADING;
    ctx.loadStep = 0;

    pthread_t database_service_thread;
    pthread_mutex_init(&db_lock, NULL);

    pthread_create(&database_service_thread, NULL, thread_work_schedule, (void*)&ctx);

    ctx.screenWidth = GetSystemMetrics(SM_CXSCREEN);
    ctx.screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (ctx.screenWidth <= 0) ctx.screenWidth = 1920;
    if (ctx.screenHeight <= 0) ctx.screenHeight = 1080;

    ctx.panelW = 860;
    ctx.panelH = 740;
    ctx.isResizing = false;
    ctx.frameDelay = 0.033f;

    LoadSettings(&ctx);

    ctx.bgDark = (Color){ 30, 32, 38, 245 };
    ctx.bgPanel = (Color){ 45, 48, 56, 255 };
    ctx.textSoft = (Color){ 220, 225, 230, 255 };
    ctx.accentGreen = (Color){ 80, 200, 140, 255 };
    ctx.accentGold = (Color){ 240, 200, 90, 255 };
    ctx.btnNormal = (Color){ 70, 75, 85, 255 };
    ctx.btnHover = (Color){ 100, 105, 115, 255 };
    ctx.activeBox = (Color){ 60, 65, 75, 255 };

    FetchWindowsUsername(&ctx);

    SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT);
    InitWindow(ctx.screenWidth, ctx.screenHeight - 1, APP_WINDOW_TITLE);
    SetWindowPosition(0, 0);
    DisableEventWaiting();
    SetTargetFPS(60);
    SetExitKey(0);

    ctx.hwnd = (HWND)GetWindowHandle();
    if (ctx.hwnd != NULL) {
        LONG_PTR exStyle = GetWindowLongPtr(ctx.hwnd, GWL_EXSTYLE);
        SetWindowLongPtr(ctx.hwnd, GWL_EXSTYLE, (exStyle & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);
    }

    InitAudioDevice();

    double previousTime = GetTime();

    while (!WindowShouldClose() && ctx.isRunning) {
        double currentTime = GetTime();
        float dt = (float)(currentTime - previousTime);
        previousTime = currentTime;

        switch (ctx.currentState) {
        case STATE_LOADING:    UpdateLoadingState(&ctx, dt); break;
        case STATE_SETUP:      UpdateSetupState(&ctx); break;
        case STATE_PROMPT_DB:  UpdatePromptDbState(&ctx); break;
        case STATE_SCHEDULE_DB:UpdateScheduleState(&ctx); break;
        case STATE_WORKING:    UpdateWorkingState(&ctx, dt); break;
        case STATE_WARNING:    UpdateWarningState(&ctx, dt); break;
        case STATE_STATUS:     UpdateStatusState(&ctx, dt); break;
        case STATE_SCHEDULE_DETAIL: UpdateScheduleDetailState(&ctx); break;
        case STATE_LOCKED:     UpdateLockedState(&ctx, dt); break;
        case STATE_BREAK_OVER: UpdateBreakOverState(&ctx, dt); break;
        case STATE_FINISHED:   UpdateFinishedState(&ctx, dt); break;
        }

        BeginDrawing();
        ClearBackground(BLANK);

        switch (ctx.currentState) {
        case STATE_LOADING:    DrawLoadingState(&ctx); break;
        case STATE_SETUP:      DrawSetupState(&ctx); break;
        case STATE_PROMPT_DB:  DrawPromptDbState(&ctx); break;
        case STATE_SCHEDULE_DB:DrawScheduleState(&ctx); break;
        case STATE_WORKING:    break;
        case STATE_WARNING:    DrawWarningState(&ctx); break;
        case STATE_STATUS:     DrawStatusState(&ctx); break;
        case STATE_LOCKED:     DrawLockedState(&ctx); break;
        case STATE_SCHEDULE_DETAIL: DrawScheduleDetailState(&ctx); break;
        case STATE_BREAK_OVER: DrawBreakOverState(&ctx); break;
        case STATE_FINISHED:   DrawFinishedState(&ctx); break;
        }

        EndDrawing();
    }

    CleanupSystemLocks();
    UnloadAppAssets(&ctx);

    if (ctx.dbHandle != NULL) {
        destroy_work_schedule_wrapper(ctx.dbHandle);
    }

    CloseAudioDevice();
    CloseWindow();

    pthread_join(database_service_thread, NULL);
    pthread_mutex_destroy(&db_lock);

    return 0;
}