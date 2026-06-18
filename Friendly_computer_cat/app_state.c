#include "app.h"

static void UpdateGifFrame(Image* im, Texture2D* tex, int frames, int* currentFrame, float* frameTimer, float frameDelay, float dt) {
    if (im->data == NULL || frames <= 0) return;

    *frameTimer += dt;
    if (*frameTimer >= frameDelay) {
        *currentFrame = (*currentFrame + 1) % frames; 
        unsigned int nextOffset = im->width * im->height * 4 * (*currentFrame);
        UpdateTexture(*tex, ((unsigned char*)im->data) + nextOffset);
        *frameTimer = 0.0f;
    }
}

static void UpdateTrayTimer(AppContext* ctx) {
    static int lastSec = -1;
    int currentSec = (int)ctx->workTimer;

    if (currentSec != lastSec && currentSec >= 0) {
        char tip[64];
        sprintf_s(tip, sizeof(tip), "Focus Time: %02d:%02d left", currentSec / 60, currentSec % 60);
        UpdateTrayIconTooltip(ctx->hwnd, tip);
        lastSec = currentSec;
    }

    if (ctx->workTimer <= 0.0f) lastSec = -1;
}


void UpdateLoadingState(AppContext* ctx, float dt) {
    if (ctx == NULL) return;

    bool isFinished = UpdateAssetLoading(ctx);
    if (isFinished) {
        ctx->currentState = STATE_SETUP;
    }
}

void UpdateSetupState(AppContext* ctx) {
    if (ctx == NULL) return;

    if (ctx->showHelpScreen) {
        if (IsKeyPressed(KEY_ESCAPE)) ctx->showHelpScreen = false;
        return;
    }

    int remLen = (int)strlen(ctx->reminderMessage);
    int key = GetCharPressed();

    while (key > 0) {
        if (ctx->activeField == 0) {
            if ((key >= 32) && (key <= 125) && (ctx->nameLength < 63)) {
                ctx->userName[ctx->nameLength] = (char)key;
                ctx->userName[ctx->nameLength + 1] = '\0';
                ctx->nameLength++;
            }
        }
        else if (ctx->activeField == 1) {
            if ((key >= 32) && (key <= 125) && (remLen < 255)) {
                ctx->reminderMessage[remLen] = (char)key;
                ctx->reminderMessage[remLen + 1] = '\0';
                remLen++;
            }
        }
        else if (key >= '0' && key <= '9') {
            int num = key - '0';

            if (ctx->activeField == 2) {
                ctx->workMin = ctx->workMin * 10 + num;
                if (ctx->workMin > 99) ctx->workMin = num;
            }
            else if (ctx->activeField == 3) {
                ctx->workSec = ctx->workSec * 10 + num;
                if (ctx->workSec > 59) ctx->workSec = num;
            }
            else if (ctx->activeField == 4) {
                ctx->breakMin = ctx->breakMin * 10 + num;
                if (ctx->breakMin > 99) ctx->breakMin = num;
            }
            else if (ctx->activeField == 5) {
                ctx->breakSec = ctx->breakSec * 10 + num;
                if (ctx->breakSec > 59) ctx->breakSec = num;
            }
            else if (ctx->activeField == 6) {
                ctx->warningSec = ctx->warningSec * 10 + num;
                if (ctx->warningSec > 60) ctx->warningSec = num;
            }
            else if (ctx->activeField == 7) {
                ctx->totalCycles = ctx->totalCycles * 10 + num;
                if (ctx->totalCycles > 99) ctx->totalCycles = num;
            }
            else if (ctx->activeField == 8) {
                ctx->volumeLevel = ctx->volumeLevel * 10 + num;
                if (ctx->volumeLevel > 100) ctx->volumeLevel = num;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (ctx->activeField == 0 && ctx->nameLength > 0) {
            ctx->nameLength--;
            ctx->userName[ctx->nameLength] = '\0';
        }
        else if (ctx->activeField == 1 && remLen > 0) {
            remLen--;
            ctx->reminderMessage[remLen] = '\0';
        }
        else if (ctx->activeField == 2) ctx->workMin /= 10;
        else if (ctx->activeField == 3) ctx->workSec /= 10;
        else if (ctx->activeField == 4) ctx->breakMin /= 10;
        else if (ctx->activeField == 5) ctx->breakSec /= 10;
        else if (ctx->activeField == 6) ctx->warningSec /= 10;
        else if (ctx->activeField == 7) ctx->totalCycles /= 10;
        else if (ctx->activeField == 8) ctx->volumeLevel /= 10;
    }

    if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN) || IsKeyPressed(KEY_TAB) || IsKeyPressedRepeat(KEY_TAB)) {
        ctx->activeField++;
        if (ctx->activeField > 8) ctx->activeField = 0;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP)) {
        ctx->activeField--;
        if (ctx->activeField < 0) ctx->activeField = 8;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        ctx->workTimer = (float)(ctx->workMin * 60 + ctx->workSec);
        if (ctx->workTimer < 1.0f) ctx->workTimer = 1.0f;

        SetSoundVolume(ctx->alarmSound, (float)ctx->volumeLevel / 100.0f);
        ctx->currentCycle = 1;
        ctx->currentState = STATE_WORKING;

        SetExitKey(0);
        SaveSettings(ctx);
        InitSystemManager(ctx);

        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        MakeWindowClickThrough(ctx->hwnd);
        AddTrayIcon(ctx->hwnd, "Friendly Computer Cat: Started");
    }
}

void UpdateScheduleState(AppContext* ctx) {
    if (ctx == NULL) return;

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (ctx->currentScheduleList != NULL) {
            freeLinkedList(ctx->currentScheduleList);
            ctx->currentScheduleList = NULL;
        }
        ctx->currentState = STATE_SETUP;
    }
}

void UpdateWorkingState(AppContext* ctx, float dt) {
    if (ctx == NULL) return;

    ctx->workTimer -= dt;
    UpdateTrayTimer(ctx);

    if (ctx->workTimer <= (float)ctx->warningSec && ctx->workTimer > 0.0f) {
        ctx->currentState = STATE_WARNING;
        MakeWindowClickThrough(ctx->hwnd);
        SetWindowPos(ctx->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    else if (ctx->workTimer <= 0.0f) {
        RemoveTrayIcon(ctx->hwnd);
        RestoreWindowClickable(ctx->hwnd);
        SetForegroundWindow(ctx->hwnd);
        SetWindowPos(ctx->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        PlaySound(ctx->alarmSound);

        ctx->lockTimer = (float)(ctx->breakMin * 60 + ctx->breakSec);
        if (ctx->lockTimer < 1.0f) ctx->lockTimer = 1.0f;

        ctx->currentState = STATE_LOCKED;
        EnableSystemLocks();
    }
}

void UpdateWarningState(AppContext* ctx, float dt) {
    if (ctx == NULL) return;

    ctx->workTimer -= dt;
    UpdateTrayTimer(ctx);

    if (ctx->workTimer <= 0.0f) {
        RemoveTrayIcon(ctx->hwnd);
        RestoreWindowClickable(ctx->hwnd);
        SetForegroundWindow(ctx->hwnd);
        SetWindowPos(ctx->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        PlaySound(ctx->alarmSound);

        ctx->lockTimer = (float)(ctx->breakMin * 60 + ctx->breakSec);
        if (ctx->lockTimer < 1.0f) ctx->lockTimer = 1.0f;

        ctx->currentState = STATE_LOCKED;
        EnableSystemLocks();
    }
}

void UpdateStatusState(AppContext* ctx, float dt) {
    if (ctx == NULL) return;

    ctx->workTimer -= dt;
    UpdateTrayTimer(ctx);

    if (ctx->workTimer <= 0.0f) {
        RemoveTrayIcon(ctx->hwnd);
        RestoreWindowClickable(ctx->hwnd);
        SetForegroundWindow(ctx->hwnd);
        SetWindowPos(ctx->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        PlaySound(ctx->alarmSound);
        ctx->lockTimer = (float)(ctx->breakMin * 60 + ctx->breakSec);
        if (ctx->lockTimer < 1.0f) ctx->lockTimer = 1.0f;

        ctx->currentState = STATE_LOCKED;
        EnableSystemLocks();
        return;
    }

    UpdateGifFrame(&ctx->imAnim3, &ctx->texAnim3, ctx->animFrames3, &ctx->currentAnimFrame3, &ctx->frameTimer3, ctx->frameDelay, dt);

    int titleLen = (int)strlen(ctx->promptTitle);
    int infoLen = (int)strlen(ctx->promptInfo);
    int key = GetCharPressed();

    while (key > 0) {
        if (ctx->activeField == 0) {
            if ((key >= 32) && (key <= 125) && (titleLen < 63)) {
                ctx->promptTitle[titleLen] = (char)key;
                ctx->promptTitle[titleLen + 1] = '\0';
                titleLen++;
            }
        }
        else if (ctx->activeField == 1) {
            if ((key >= 32) && (key <= 125) && (infoLen < 255)) {
                ctx->promptInfo[infoLen] = (char)key;
                ctx->promptInfo[infoLen + 1] = '\0';
                infoLen++;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (ctx->activeField == 0 && titleLen > 0) ctx->promptTitle[titleLen - 1] = '\0';
        else if (ctx->activeField == 1 && infoLen > 0) ctx->promptInfo[infoLen - 1] = '\0';
    }

    if (IsKeyPressed(KEY_TAB)) {
        ctx->activeField = (ctx->activeField == 0) ? 1 : 0;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (ctx->workTimer <= (float)ctx->warningSec) ctx->currentState = STATE_WARNING;
        else ctx->currentState = STATE_WORKING;
        MakeWindowClickThrough(ctx->hwnd);
    }
}

void UpdateLockedState(AppContext* ctx, float dt) {
    if (ctx == NULL) return;

    ctx->lockTimer -= dt;

    UpdateGifFrame(&ctx->imAnim1, &ctx->texAnim1, ctx->animFrames1, &ctx->currentAnimFrame1, &ctx->frameTimer1, ctx->frameDelay, dt);

    if (ctx->lockTimer <= 0.0f) {
        PlaySound(ctx->alarmSound);

        if (ctx->currentCycle >= ctx->totalCycles) {
            ctx->finishedTimer = 5.0f;
            ctx->currentState = STATE_FINISHED;
        }
        else {
            ctx->breakOverTimer = 4.0f;
            ctx->currentState = STATE_BREAK_OVER;
        }

        ShowCursor();
        CleanupSystemLocks();
    }
}

void UpdateBreakOverState(AppContext* ctx, float dt) {
    if (ctx == NULL) return;

    ctx->breakOverTimer -= dt;

    UpdateGifFrame(&ctx->imAnim2, &ctx->texAnim2, ctx->animFrames2, &ctx->currentAnimFrame2, &ctx->frameTimer2, ctx->frameDelay, dt);

    if (ctx->breakOverTimer <= 0.0f) {
        ctx->currentCycle++;
        ctx->workTimer = (float)(ctx->workMin * 60 + ctx->workSec);

        if (ctx->workTimer <= (float)ctx->warningSec) {
            ctx->currentState = STATE_WARNING;
        }
        else {
            ctx->currentState = STATE_WORKING;
        }
        MakeWindowClickThrough(ctx->hwnd);
        AddTrayIcon(ctx->hwnd, "Friendly Computer Cat: Focusing...");
    }
}

void UpdateFinishedState(AppContext* ctx, float dt) {
    if (ctx == NULL) return;

    ctx->finishedTimer -= dt;

    UpdateGifFrame(&ctx->imAnim4, &ctx->texAnim4, ctx->animFrames4, &ctx->currentAnimFrame4, &ctx->frameTimer4, ctx->frameDelay, dt);

    if (ctx->finishedTimer <= 0.0f) {
        ctx->isRunning = false;
    }
}

void UpdatePromptDbState(AppContext* ctx) {
    if (ctx == NULL) return;

    if (IsKeyPressed(KEY_ESCAPE)) {
        ctx->currentState = STATE_SETUP;
        return;
    }

    int titleLen = (int)strlen(ctx->promptTitle);
    int infoLen = (int)strlen(ctx->promptInfo);
    int key = GetCharPressed();

    while (key > 0) {
        if (ctx->promptActiveField == 0) { 
            if ((key >= 32) && (key <= 125) && (titleLen < 63)) {
                ctx->promptTitle[titleLen] = (char)key;
                ctx->promptTitle[titleLen + 1] = '\0';
                titleLen++;
            }
        }
        else if (ctx->promptActiveField == 1) { 
            if ((key >= 32) && (key <= 125) && (infoLen < 255)) {
                ctx->promptInfo[infoLen] = (char)key;
                ctx->promptInfo[infoLen + 1] = '\0';
                infoLen++;
            }
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        if (ctx->promptActiveField == 0 && titleLen > 0) {
            ctx->promptTitle[titleLen - 1] = '\0';
        }
        else if (ctx->promptActiveField == 1 && infoLen > 0) {
            ctx->promptInfo[infoLen - 1] = '\0';
        }
    }

    if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP)) {
        ctx->promptActiveField = (ctx->promptActiveField == 0) ? 1 : 0; 
    }
}

void UpdateScheduleDetailState(AppContext* ctx) {
    if (ctx == NULL) return;

    if (IsKeyPressed(KEY_ESCAPE)) {
        ctx->currentState = STATE_SCHEDULE_DB; 
    }
}