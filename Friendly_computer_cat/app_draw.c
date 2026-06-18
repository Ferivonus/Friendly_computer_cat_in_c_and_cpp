#include "app.h"

#define UI_RADIUS 0.15f
#define UI_SEGS 16
#define BTN_FONT_SIZE 18

static AppState lastState = STATE_SETUP;
static bool lastHelpState = false;
static float fadeAlpha = 0.0f;

typedef struct {
    float x;
    float y;
    float w;
    float h;
} UICard;

static UICard DrawGlassCard(float yPos, float width, float height, Color bg, Color border, int screenWidth) {
    UICard card = { 0 };
    card.w = width;
    card.h = height;
    card.x = ((float)screenWidth - width) / 2.0f;
    card.y = yPos;

    DrawRectangleRounded((Rectangle) { card.x, card.y, card.w, card.h }, UI_RADIUS, UI_SEGS, bg);
    DrawRectangleRoundedLines((Rectangle) { card.x, card.y, card.w, card.h }, UI_RADIUS, UI_SEGS, border);

    return card;
}

static void DrawCardText(UICard card, const char* text, int fontSize, Color color, float offsetY) {
    if (!text) return;
    int textW = MeasureText(text, fontSize);
    DrawText(text, (int)(card.x + (card.w - (float)textW) / 2.0f), (int)(card.y + offsetY), fontSize, color);
}

static void HandleTransitions(AppContext* ctx) {
    if (ctx == NULL) return;
    if (ctx->currentState == STATE_WORKING || ctx->currentState == STATE_WARNING) return;

    if (ctx->currentState != lastState || ctx->showHelpScreen != lastHelpState) {
        fadeAlpha = 0.3f;
        lastState = ctx->currentState;
        lastHelpState = ctx->showHelpScreen;
    }

    if (fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, ctx->screenWidth, ctx->screenHeight, Fade(ctx->bgDark, fadeAlpha));
        fadeAlpha -= GetFrameTime() * 5.0f;
        if (fadeAlpha < 0.0f) fadeAlpha = 0.0f;
    }
}

bool DrawButtonAdvanced(Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color textColor, bool isEnabled) {
    if (text == NULL) return false;

    Vector2 mouse = GetMousePosition();
    bool isHover = isEnabled && CheckCollisionPointRec(mouse, rect);
    bool isClicked = isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    Color bgColor = isEnabled ? (isHover ? hoverColor : baseColor) : Fade(baseColor, 0.5f);
    Color tColor = isEnabled ? textColor : Fade(textColor, 0.5f);

    DrawRectangleRounded(rect, UI_RADIUS, UI_SEGS, bgColor);

    int tw = MeasureText(text, BTN_FONT_SIZE);
    DrawText(text, (int)(rect.x + (rect.width - (float)tw) / 2.0f), (int)(rect.y + (rect.height - (float)BTN_FONT_SIZE) / 2.0f), BTN_FONT_SIZE, tColor);

    return isClicked;
}

bool DrawButton(Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color textColor) {
    return DrawButtonAdvanced(rect, text, baseColor, hoverColor, textColor, true);
}

static bool DrawButtonRepeat(Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color textColor) {
    if (text == NULL) return false;

    Vector2 mouse = GetMousePosition();
    bool isHover = CheckCollisionPointRec(mouse, rect);
    bool isPressed = isHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    bool isDown = isHover && IsMouseButtonDown(MOUSE_LEFT_BUTTON);

    static float holdTimer = 0.0f;
    static Rectangle activeRect = { 0 };
    bool trigger = false;

    if (isPressed) {
        holdTimer = 0.0f;
        activeRect = rect;
        trigger = true;
    }
    else if (isDown && activeRect.x == rect.x && activeRect.y == rect.y) {
        holdTimer += GetFrameTime();
        if (holdTimer > 0.4f) {
            holdTimer -= 0.1f;
            trigger = true;
        }
    }
    else if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (activeRect.x == rect.x && activeRect.y == rect.y) {
            activeRect = (Rectangle){ 0 };
        }
    }

    Color bgColor = (isHover && isDown) ? hoverColor : (isHover ? hoverColor : baseColor);
    DrawRectangleRounded(rect, UI_RADIUS, UI_SEGS, bgColor);

    int tw = MeasureText(text, BTN_FONT_SIZE);
    DrawText(text, (int)(rect.x + (rect.width - (float)tw) / 2.0f), (int)(rect.y + (rect.height - (float)BTN_FONT_SIZE) / 2.0f), BTN_FONT_SIZE, textColor);

    return trigger;
}

void DrawLoadingState(AppContext* ctx) {
    if (ctx == NULL || ctx->screenWidth <= 0) return;

    float boxW = 500.0f, boxH = 200.0f;
    float boxY = (float)(ctx->screenHeight - (int)boxH) / 2.0f;

    UICard card = DrawGlassCard(boxY, boxW, boxH, ctx->bgDark, ctx->btnNormal, ctx->screenWidth);
    DrawCardText(card, "FRIENDLY COMPUTER CAT", 24, ctx->accentGreen, 40.0f);

    const char* messages[] = {
        "Loading audio files...", "Loading work animation... (1/4)",
        "Loading break animation... (2/4)", "Loading info animation... (3/4)",
        "Loading closing animation... (4/4)", "Starting system..."
    };

    int step = ctx->loadStep > 5 ? 5 : ctx->loadStep;
    DrawCardText(card, messages[step], 16, ctx->textSoft, 90.0f);

    float progress = (float)step / 5.0f;
    float barW = 420.0f;
    float barX = card.x + (card.w - barW) / 2.0f;
    float barY = card.y + 140.0f;

    DrawRectangleRounded((Rectangle) { barX, barY, barW, 8.0f }, 0.5f, UI_SEGS, ctx->bgPanel);
    DrawRectangleRounded((Rectangle) { barX, barY, barW* progress, 8.0f }, 0.5f, UI_SEGS, ctx->accentGold);
}

void DrawWarningState(AppContext* ctx) {
    if (ctx == NULL || ctx->screenWidth <= 0) return;

    int sec = (int)ctx->workTimer;
    if (sec < 0) sec = 0;

    const char* warnMsg = TextFormat("Get ready! Break starts in %d seconds", sec);
    float boxW = (float)MeasureText(warnMsg, 24) + 80.0f;

    UICard card = DrawGlassCard(60.0f, boxW, 60.0f, (Color) { 30, 32, 38, 230 }, ctx->accentGold, ctx->screenWidth);
    DrawCardText(card, warnMsg, 24, ctx->accentGold, 18.0f);
}

void DrawStatusState(AppContext* ctx) {
    if (ctx == NULL || ctx->screenWidth <= 0 || ctx->screenHeight <= 0) return;

    float boxW = 520.0f, boxH = 540.0f;
    int spacing = 25;

    int gifW = (ctx->imAnim3.data != NULL) ? ctx->imAnim3.width : 0;
    int gifH = (ctx->imAnim3.data != NULL) ? ctx->imAnim3.height : 0;

    int totalHeight = gifH + (gifH > 0 ? spacing : 0) + (int)boxH;
    int startY = (ctx->screenHeight - totalHeight) / 2;
    int gifX = (ctx->screenWidth - gifW) / 2;
    int gifY = startY;

    if (ctx->imAnim3.data != NULL) DrawTexture(ctx->texAnim3, gifX, gifY, WHITE);

    float boxY = (float)((gifH > 0) ? (gifY + gifH + spacing) : startY);
    UICard card = DrawGlassCard(boxY, boxW, boxH, Fade(ctx->bgDark, 0.95f), ctx->btnNormal, ctx->screenWidth);

    DrawCardText(card, "CURRENT SESSION", 22, ctx->textSoft, 25.0f);
    DrawLine(card.x + 50, card.y + 60, card.x + card.w - 50, card.y + 60, ctx->bgPanel);

    int m = (int)ctx->workTimer / 60;
    int s = (int)ctx->workTimer % 60;
    const char* timeText = TextFormat("%02d:%02d", m, s);
    DrawCardText(card, timeText, 72, WHITE, 80.0f);

    const char* targetText = TextFormat("Target: %02d:%02d", ctx->workMin, ctx->workSec);
    DrawCardText(card, targetText, 16, ctx->accentGold, 165.0f);

    float totalWork = (float)(ctx->workMin * 60 + ctx->workSec);
    float progress = (totalWork > 0.0f) ? (1.0f - (ctx->workTimer / totalWork)) : 0.0f;
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    float pBarW = card.w - 100.0f;
    float pBarX = card.x + 50.0f;
    float pBarY = card.y + 200.0f;

    DrawRectangleRounded((Rectangle) { pBarX, pBarY, pBarW, 14.0f }, 0.5f, UI_SEGS, ctx->bgPanel);
    DrawRectangleRounded((Rectangle) { pBarX, pBarY, pBarW* progress, 14.0f }, 0.5f, UI_SEGS, ctx->accentGreen);

    Vector2 mouse = GetMousePosition();
    bool showCursor = ((int)(GetTime() * 2.5f) % 2) == 0;
    float fieldX = card.x + 50.0f;
    float fieldW = card.w - 100.0f;

    DrawText("Task Title", (int)fieldX, (int)card.y + 245, 14, Fade(ctx->textSoft, 0.6f));
    Rectangle titleRect = { fieldX, card.y + 265, fieldW, 40.0f };
    if (CheckCollisionPointRec(mouse, titleRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 0;
    DrawRectangleRounded(titleRect, UI_RADIUS, UI_SEGS, ctx->activeField == 0 ? ctx->activeBox : ctx->bgPanel);
    DrawText(ctx->promptTitle, (int)(titleRect.x + 15), (int)(titleRect.y + 11), 18, ctx->activeField == 0 ? ctx->accentGold : WHITE);
    if (ctx->activeField == 0 && showCursor) DrawText("|", (int)(titleRect.x + 15) + MeasureText(ctx->promptTitle, 18) + 2, (int)(titleRect.y + 11), 18, ctx->accentGold);

    DrawText("Task Description", (int)fieldX, (int)card.y + 320, 14, Fade(ctx->textSoft, 0.6f));
    Rectangle infoRect = { fieldX, card.y + 340, fieldW, 40.0f };
    if (CheckCollisionPointRec(mouse, infoRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 1;
    DrawRectangleRounded(infoRect, UI_RADIUS, UI_SEGS, ctx->activeField == 1 ? ctx->activeBox : ctx->bgPanel);
    DrawText(ctx->promptInfo, (int)(infoRect.x + 15), (int)(infoRect.y + 11), 18, ctx->activeField == 1 ? ctx->accentGold : WHITE);
    if (ctx->activeField == 1 && showCursor) DrawText("|", (int)(infoRect.x + 15) + MeasureText(ctx->promptInfo, 18) + 2, (int)(infoRect.y + 11), 18, ctx->accentGold);

    float btnW = 195.0f;
    float btnGap = 30.0f;
    float startX = card.x + (card.w - (btnW * 2.0f + btnGap)) / 2.0f;
    float row1Y = card.y + 415.0f;
    float row2Y = card.y + 470.0f;

    Rectangle btnResume = { startX, row1Y, btnW, 42.0f };
    Rectangle btnUpdate = { startX + btnW + btnGap, row1Y, btnW, 42.0f };
    Rectangle btnAddMin = { startX, row2Y, btnW, 42.0f };
    Rectangle btnAbort = { startX + btnW + btnGap, row2Y, btnW, 42.0f };

    if (DrawButton(btnResume, "RESUME", ctx->accentGreen, ctx->btnHover, ctx->bgDark)) {
        if (ctx->workTimer <= (float)ctx->warningSec) ctx->currentState = STATE_WARNING;
        else ctx->currentState = STATE_WORKING;
        MakeWindowClickThrough(ctx->hwnd);
    }

    if (DrawButton(btnUpdate, "SAVE EDITS", ctx->btnNormal, ctx->btnHover, ctx->accentGold)) {
        HandleUpdateSchedule(ctx);
        ctx->activeField = -1;
    }

    if (ctx->add_button_Clicked.howManyTimes) {
        DrawButtonAdvanced(btnAddMin, "MAX LIMIT REACHED", ctx->bgPanel, ctx->btnHover, Fade(ctx->textSoft, 0.4f), false);
    }
    else {
        const char* addText = ctx->add_button_Clicked.clicked ? "+ 5 MINUTES (LAST)" : "+ 5 MINUTES";
        if (DrawButtonAdvanced(btnAddMin, addText, ctx->btnNormal, ctx->btnHover, ctx->textSoft, true)) {
            ctx->workTimer += 5 * 60;
            ctx->workMin += 5;
            if (ctx->workTimer <= (float)ctx->warningSec) ctx->currentState = STATE_WARNING;
            if (ctx->add_button_Clicked.clicked) ctx->add_button_Clicked.howManyTimes = true;
            else ctx->add_button_Clicked.clicked = true;
        }
    }

    if (DrawButton(btnAbort, "CANCEL SESSION", (Color) { 180, 60, 60, 255 }, (Color) { 220, 80, 80, 255 }, WHITE)) {
        if (ctx->add_button_Clicked.howManyTimes) ctx->workMin -= 10;
        else if (ctx->add_button_Clicked.clicked) ctx->workMin -= 5;
        ctx->add_button_Clicked.clicked = false;
        ctx->add_button_Clicked.howManyTimes = false;

        StopSound(ctx->alarmSound);
        RemoveTrayIcon(ctx->hwnd);
        RestoreWindowClickable(ctx->hwnd);
        SetForegroundWindow(ctx->hwnd);
        ctx->currentState = STATE_SETUP;
    }

    HandleTransitions(ctx);
}

void DrawLockedState(AppContext* ctx) {
    if (ctx == NULL || ctx->screenWidth <= 0 || ctx->screenHeight <= 0) return;

    int gifX = (ctx->screenWidth - ctx->imAnim1.width) / 2;
    int gifY = (ctx->screenHeight - ctx->imAnim1.height) / 2;

    if (ctx->imAnim1.data != NULL) DrawTexture(ctx->texAnim1, gifX, gifY, WHITE);

    int min = (int)ctx->lockTimer / 60;
    int sec = (int)ctx->lockTimer % 60;

    const char* quote = ctx->motivationalQuotes[(ctx->currentCycle - 1) % QUOTE_COUNT];
    const char* greeting = TextFormat("Keep going, %s!", ctx->userName);
    const char* info = TextFormat("CYCLE: %d / %d", ctx->currentCycle, ctx->totalCycles);
    const char* timerText = TextFormat("%02d : %02d", min, sec);

    int maxW = MeasureText(timerText, 80);
    if (MeasureText(info, 28) > maxW) maxW = MeasureText(info, 28);
    if (MeasureText(greeting, 32) > maxW) maxW = MeasureText(greeting, 32);
    if (MeasureText(quote, 24) > maxW) maxW = MeasureText(quote, 24);

    float boxHeight = 260.0f;
    float boxY = (float)((ctx->imAnim1.data != NULL) ? (gifY - 280) : (ctx->screenHeight / 2 - (int)boxHeight / 2));

    UICard card = DrawGlassCard(boxY, (float)maxW + 120.0f, boxHeight, Fade(ctx->bgDark, 0.95f), ctx->btnNormal, ctx->screenWidth);

    DrawCardText(card, greeting, 32, ctx->accentGold, 30.0f);
    DrawCardText(card, quote, 24, ctx->textSoft, 80.0f);
    DrawCardText(card, info, 28, ctx->accentGreen, 130.0f);
    DrawCardText(card, timerText, 80, WHITE, 170.0f);

    HandleTransitions(ctx);
}

void DrawBreakOverState(AppContext* ctx) {
    if (ctx == NULL || ctx->screenWidth <= 0 || ctx->screenHeight <= 0) return;

    int gif2X = (ctx->screenWidth - ctx->imAnim2.width) / 2;
    int gif2Y = (ctx->screenHeight - ctx->imAnim2.height) / 2;

    if (ctx->imAnim2.data != NULL) DrawTexture(ctx->texAnim2, gif2X, gif2Y, WHITE);

    const char* msg = ctx->reminderMessage;
    const char* cycleInfo = TextFormat("Cycle %d is starting...", ctx->currentCycle + 1);

    float boxW = (float)MeasureText(msg, 48) + 100.0f;
    if (boxW < 450.0f) boxW = 450.0f;

    float boxH = 160.0f;
    float boxY = (float)((ctx->imAnim2.data != NULL) ? (gif2Y - 40 - (int)boxH) : (ctx->screenHeight / 2 - (int)boxH / 2));

    UICard card = DrawGlassCard(boxY, boxW, boxH, Fade((Color) { 20, 25, 30, 255 }, 0.95f), ctx->accentGreen, ctx->screenWidth);

    DrawCardText(card, msg, 48, ctx->accentGreen, 40.0f);
    if (ctx->currentCycle < ctx->totalCycles) {
        DrawCardText(card, cycleInfo, 26, ctx->accentGold, 100.0f);
    }

    HandleTransitions(ctx);
}

void DrawFinishedState(AppContext* ctx) {
    if (ctx == NULL || ctx->screenWidth <= 0 || ctx->screenHeight <= 0) return;

    int gifX = (ctx->screenWidth - ctx->imAnim4.width) / 2;
    int gifY = (ctx->screenHeight - ctx->imAnim4.height) / 2;

    if (ctx->imAnim4.data != NULL) DrawTexture(ctx->texAnim4, gifX, gifY, WHITE);

    const char* msg = "Congratulations!";
    const char* subMsg = "All sessions complete. Enjoy your rest!";

    float boxW = (float)MeasureText(msg, 56) + 120.0f;
    float boxH = 180.0f;
    float boxY = (float)((ctx->imAnim4.data != NULL) ? (gifY - 60 - (int)boxH) : (ctx->screenHeight / 2 - (int)boxH / 2));

    UICard card = DrawGlassCard(boxY, boxW, boxH, Fade((Color) { 30, 25, 20, 255 }, 0.95f), ctx->accentGold, ctx->screenWidth);

    DrawCardText(card, msg, 56, ctx->accentGold, 40.0f);
    DrawCardText(card, subMsg, 28, ctx->accentGreen, 110.0f);

    HandleTransitions(ctx);
}

void DrawScheduleState(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL) return;

    int panelX = (ctx->screenWidth - ctx->panelW) / 2;
    int panelY = (ctx->screenHeight - ctx->panelH) / 2;
    DrawRectangleRounded((Rectangle) { (float)panelX, (float)panelY, (float)ctx->panelW, (float)ctx->panelH }, 0.05f, 20, ctx->bgDark);

    const char* title = "WORK SCHEDULE HISTORY";
    DrawText(title, panelX + (ctx->panelW - MeasureText(title, 32)) / 2, panelY + 40, 32, ctx->accentGold);
    DrawLine(panelX + 50, panelY + 90, panelX + ctx->panelW - 50, panelY + 90, ctx->btnNormal);

    Rectangle backBtn = { (float)panelX + 40.0f, (float)panelY + 35.0f, 110.0f, 40.0f };
    if (DrawButton(backBtn, "< BACK", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) {
        if (ctx->currentScheduleList != NULL) {
            freeLinkedList(ctx->currentScheduleList);
            ctx->currentScheduleList = NULL;
        }
        ctx->currentState = STATE_SETUP;
        return;
    }

    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0.0f) {
        ctx->scheduleScrollY += (int)(wheelMove * 45.0f);
        if (ctx->scheduleScrollY > 0) ctx->scheduleScrollY = 0;
    }

    float listY = panelY + 120.0f + ctx->scheduleScrollY;
    float cardW = ctx->panelW - 100.0f;
    float cardX = panelX + 50.0f;

    BeginScissorMode(panelX, panelY + 110, ctx->panelW, ctx->panelH - 220);

    if (ctx->currentScheduleList == NULL || ctx->currentScheduleList->head == NULL) {
        DrawText("No history found. Start a new focus session to record data.", (int)cardX + 20, panelY + 160, 22, ctx->textSoft);
    }
    else {
        struct node* current = ctx->currentScheduleList->head;
        int index = 0;

        while (current != NULL) {
            C_WorkScheduleItem* item = &(current->data);

            if (item != NULL) {
                float currentCardY = listY + (index * 80.0f);

                if (currentCardY > panelY + 30 && currentCardY < panelY + ctx->panelH - 120) {
                    Rectangle itemRect = { cardX, currentCardY, cardW, 65.0f };

                    Color itemBg = ctx->bgPanel;
                    if (CheckCollisionPointRec(GetMousePosition(), itemRect)) {
                        itemBg = ctx->btnHover;
                        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);

                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            ctx->selectedScheduleId = item->id;
                            ctx->viewedSchedule = item;
                            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                            ctx->currentState = STATE_SCHEDULE_DETAIL;
                        }
                    }
                    else {
                        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    }

                    DrawRectangleRounded(itemRect, UI_RADIUS, UI_SEGS, itemBg);

                    const char* wTitle = (item->work_title != NULL && item->work_title[0] != '\0') ? item->work_title : "Untitled Task";
                    DrawText(wTitle, (int)itemRect.x + 25, (int)itemRect.y + 22, 22, WHITE);
                    DrawText(TextFormat("Duration: %d min", item->working_time_as_minute), (int)itemRect.x + (int)cardW - 180, (int)itemRect.y + 24, 18, ctx->accentGold);
                }
            }
            current = current->next;
            index++;
        }

        int maxScroll = -((index * 80) - (ctx->panelH - 260));
        if (maxScroll > 0) maxScroll = 0;
        if (ctx->scheduleScrollY < maxScroll) ctx->scheduleScrollY = maxScroll;
    }
    EndScissorMode();

    float bottomY = panelY + ctx->panelH - 90.0f;
    Rectangle delBtn = { panelX + (ctx->panelW - 200.0f) / 2.0f, bottomY, 200.0f, 50.0f };
    bool canDelete = (ctx->selectedScheduleId != -1);

    if (DrawButtonAdvanced(delBtn, "DELETE LOG", (Color) { 180, 60, 60, 255 }, (Color) { 220, 80, 80, 255 }, WHITE, canDelete)) {
        HandleDeleteSchedule(ctx);
    }

    HandleTransitions(ctx);
}

void DrawScheduleDetailState(AppContext* ctx) {
    if (ctx == NULL || ctx->viewedSchedule == NULL) return;

    int panelX = (ctx->screenWidth - ctx->panelW) / 2;
    int panelY = (ctx->screenHeight - ctx->panelH) / 2;
    DrawRectangleRounded((Rectangle) { (float)panelX, (float)panelY, (float)ctx->panelW, (float)ctx->panelH }, 0.05f, 20, ctx->bgDark);

    const char* title = "HISTORY DETAILS";
    DrawText(title, panelX + (ctx->panelW - MeasureText(title, 32)) / 2, panelY + 40, 32, ctx->accentGold);
    DrawLine(panelX + 50, panelY + 90, panelX + ctx->panelW - 50, panelY + 90, ctx->btnNormal);

    Rectangle backBtn = { (float)panelX + 40.0f, (float)panelY + 35.0f, 110.0f, 40.0f };
    if (DrawButton(backBtn, "< BACK", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) {
        ctx->currentState = STATE_SCHEDULE_DB;
        return;
    }

    C_WorkScheduleItem* item = ctx->viewedSchedule;
    float contentX = panelX + 60.0f;
    float contentY = panelY + 130.0f;
    float boxW = ctx->panelW - 120.0f;

    DrawText("Task Title", (int)contentX, (int)contentY, 18, Fade(ctx->textSoft, 0.7f));
    DrawRectangleRounded((Rectangle) { contentX, contentY + 25, boxW, 50 }, UI_RADIUS, UI_SEGS, ctx->bgPanel);
    DrawText((item->work_title && item->work_title[0] != '\0') ? item->work_title : "Untitled Task", (int)contentX + 15, (int)contentY + 40, 24, ctx->accentGold);

    contentY += 100.0f;

    DrawText("Description", (int)contentX, (int)contentY, 18, Fade(ctx->textSoft, 0.7f));
    DrawRectangleRounded((Rectangle) { contentX, contentY + 25, boxW, 50 }, UI_RADIUS, UI_SEGS, ctx->bgPanel);
    DrawText((item->work_info && item->work_info[0] != '\0') ? item->work_info : "No description provided.", (int)contentX + 15, (int)contentY + 40, 20, WHITE);

    contentY += 100.0f;

    DrawText("Session Settings", (int)contentX, (int)contentY, 18, Fade(ctx->textSoft, 0.7f));
    DrawRectangleRounded((Rectangle) { contentX, contentY + 25, boxW, 50 }, UI_RADIUS, UI_SEGS, ctx->bgPanel);
    const char* durText = TextFormat("Duration: %d Minutes   |   Cycles: %d", item->working_time_as_minute, item->how_many_turns_work);
    DrawText(durText, (int)contentX + 15, (int)contentY + 40, 20, ctx->accentGreen);

    contentY += 100.0f;

    DrawText("Created At", (int)contentX, (int)contentY, 18, Fade(ctx->textSoft, 0.7f));
    DrawRectangleRounded((Rectangle) { contentX, contentY + 25, boxW, 50 }, UI_RADIUS, UI_SEGS, ctx->bgPanel);
    DrawText((item->starting_time && item->starting_time[0] != '\0') ? item->starting_time : "Unknown", (int)contentX + 15, (int)contentY + 40, 20, WHITE);

    float bottomY = panelY + ctx->panelH - 90.0f;
    Rectangle delBtn = { panelX + (ctx->panelW - 200.0f) / 2.0f, bottomY, 200.0f, 50.0f };

    if (DrawButtonAdvanced(delBtn, "DELETE LOG", (Color) { 180, 60, 60, 255 }, (Color) { 220, 80, 80, 255 }, WHITE, true)) {
        HandleDeleteSchedule(ctx);
        ctx->currentState = STATE_SCHEDULE_DB;
    }

    HandleTransitions(ctx);
}

void DrawPromptDbState(AppContext* ctx) {
    if (ctx == NULL) return;

    DrawSetupState(ctx);
    DrawRectangle(0, 0, ctx->screenWidth, ctx->screenHeight, Fade(ctx->bgDark, 0.85f));

    float boxW = 580.0f, boxH = 420.0f;
    float boxY = (ctx->screenHeight - boxH) / 2.0f;
    UICard card = DrawGlassCard(boxY, boxW, boxH, ctx->bgPanel, ctx->accentGold, ctx->screenWidth);

    DrawCardText(card, "SAVE SCHEDULE TO DATABASE", 26, ctx->accentGold, 35.0f);
    DrawLine(card.x + 50, card.y + 80, card.x + card.w - 50, card.y + 80, ctx->btnNormal);

    bool showCursor = ((int)(GetTime() * 2.5f) % 2) == 0;
    Vector2 mouse = GetMousePosition();

    DrawText("Task Title:", (int)card.x + 50, (int)card.y + 110, 18, ctx->textSoft);
    Rectangle titleRect = { card.x + 50, card.y + 140, card.w - 100, 45.0f };
    if (CheckCollisionPointRec(mouse, titleRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->promptActiveField = 0;
    DrawRectangleRounded(titleRect, UI_RADIUS, UI_SEGS, ctx->promptActiveField == 0 ? ctx->activeBox : ctx->bgDark);
    DrawText(ctx->promptTitle, (int)(titleRect.x + 15), (int)(titleRect.y + 13), 20, ctx->promptActiveField == 0 ? ctx->accentGold : ctx->textSoft);
    if (ctx->promptActiveField == 0 && showCursor) DrawText("|", (int)(titleRect.x + 15) + MeasureText(ctx->promptTitle, 20) + 2, (int)(titleRect.y + 13), 20, ctx->accentGold);

    DrawText("Task Description:", (int)card.x + 50, (int)card.y + 210, 18, ctx->textSoft);
    Rectangle infoRect = { card.x + 50, card.y + 240, card.w - 100, 45.0f };
    if (CheckCollisionPointRec(mouse, infoRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->promptActiveField = 1;
    DrawRectangleRounded(infoRect, UI_RADIUS, UI_SEGS, ctx->promptActiveField == 1 ? ctx->activeBox : ctx->bgDark);
    DrawText(ctx->promptInfo, (int)(infoRect.x + 15), (int)(infoRect.y + 13), 20, ctx->promptActiveField == 1 ? ctx->accentGold : ctx->textSoft);
    if (ctx->promptActiveField == 1 && showCursor) DrawText("|", (int)(infoRect.x + 15) + MeasureText(ctx->promptInfo, 20) + 2, (int)(infoRect.y + 13), 20, ctx->accentGold);

    float btnW = 200.0f;
    Rectangle cancelBtn = { card.x + 60, card.y + 330, btnW, 50.0f };
    Rectangle saveBtn = { card.x + card.w - btnW - 60, card.y + 330, btnW, 50.0f };

    if (DrawButton(cancelBtn, "CANCEL", (Color) { 180, 60, 60, 255 }, (Color) { 220, 80, 80, 255 }, WHITE)) {
        ctx->currentState = STATE_SETUP;
    }

    if (DrawButton(saveBtn, "SAVE & START", ctx->accentGreen, ctx->btnHover, ctx->bgDark)) {
        HandleAddSchedule(ctx);

        ctx->workTimer = (float)(ctx->workMin * 60 + ctx->workSec);
        if (ctx->workTimer < 1.0f) ctx->workTimer = 1.0f;
        SetSoundVolume(ctx->alarmSound, (float)ctx->volumeLevel / 100.0f);
        ctx->currentCycle = 1;

        SetExitKey(0);
        SaveSettings(ctx);
        InitSystemManager(ctx);

        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        MakeWindowClickThrough(ctx->hwnd);
        AddTrayIcon(ctx->hwnd, "Friendly Computer Cat: Started");

        ctx->currentState = STATE_WORKING;
    }
}

void DrawSetupState(AppContext* ctx) {
    if (ctx == NULL || ctx->screenWidth <= 0 || ctx->screenHeight <= 0) return;

    Vector2 mouse = GetMousePosition();
    int minPanelW = 820, minPanelH = 760;

    if (ctx->targetPanelW == 0.0f) ctx->targetPanelW = (float)ctx->panelW;
    if (ctx->targetPanelH == 0.0f) ctx->targetPanelH = (float)ctx->panelH;

    int panelX = (ctx->screenWidth - ctx->panelW) / 2;
    int panelY = (ctx->screenHeight - ctx->panelH) / 2;

    Rectangle resizeArea = { (float)(panelX + ctx->panelW - 30), (float)(panelY + ctx->panelH - 30), 30.0f, 30.0f };

    if (CheckCollisionPointRec(mouse, resizeArea)) {
        SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->isResizing = true;
    }
    else if (!ctx->isResizing) SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    if (ctx->isResizing) {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            ctx->isResizing = false;
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        else {
            Vector2 delta = GetMouseDelta();
            ctx->targetPanelW += delta.x * 2.0f;
            ctx->targetPanelH += delta.y * 2.0f;
        }
    }

    if (ctx->targetPanelW > (float)(ctx->screenWidth - 40)) ctx->targetPanelW = (float)(ctx->screenWidth - 40);
    if (ctx->targetPanelH > (float)(ctx->screenHeight - 40)) ctx->targetPanelH = (float)(ctx->screenHeight - 40);
    if (ctx->targetPanelW < (float)minPanelW) ctx->targetPanelW = (float)minPanelW;
    if (ctx->targetPanelH < (float)minPanelH) ctx->targetPanelH = (float)minPanelH;

    float lerpSpeed = GetFrameTime() * 15.0f;
    ctx->panelW = (int)Lerp((float)ctx->panelW, ctx->targetPanelW, lerpSpeed);
    ctx->panelH = (int)Lerp((float)ctx->panelH, ctx->targetPanelH, lerpSpeed);

    panelX = (ctx->screenWidth - ctx->panelW) / 2;
    panelY = (ctx->screenHeight - ctx->panelH) / 2;

    bool showCursor = ((int)(GetTime() * 2.5f) % 2) == 0;

    DrawRectangleRounded((Rectangle) { (float)panelX, (float)panelY, (float)ctx->panelW, (float)ctx->panelH }, 0.05f, 20, ctx->bgDark);
    DrawLine(panelX + ctx->panelW - 20, panelY + ctx->panelH - 8, panelX + ctx->panelW - 8, panelY + ctx->panelH - 20, ctx->btnHover);
    DrawLine(panelX + ctx->panelW - 14, panelY + ctx->panelH - 8, panelX + ctx->panelW - 8, panelY + ctx->panelH - 14, ctx->btnHover);

    if (ctx->showHelpScreen) {
        const char* hTitle = "HOW IT WORKS & FEATURES";
        DrawText(hTitle, panelX + (ctx->panelW - MeasureText(hTitle, 32)) / 2, panelY + 40, 32, ctx->accentGold);
        DrawLine(panelX + 50, panelY + 90, panelX + ctx->panelW - 50, panelY + 90, ctx->btnNormal);

        float textX = panelX + 60.0f;
        float textY = panelY + 120.0f;
        float spacingY = 45.0f;

        DrawText("Welcome to Friendly Computer Cat! This app is designed to help you", (int)textX, (int)textY, 20, WHITE);
        textY += 30.0f;
        DrawText("maintain focus while ensuring you take healthy, structured breaks.", (int)textX, (int)textY, 20, WHITE);
        textY += spacingY + 15.0f;

        DrawText("Key Features:", (int)textX, (int)textY, 24, ctx->accentGreen);
        textY += 40.0f;

        DrawText("- Focus & Break Times:", (int)textX, (int)textY, 20, ctx->accentGold);
        DrawText("Customize exactly how long you want to work and rest.", (int)textX + 240, (int)textY, 20, ctx->textSoft);
        textY += spacingY;

        DrawText("- Ghost Mode:", (int)textX, (int)textY, 20, ctx->accentGold);
        DrawText("During work, the widget becomes transparent to clicks.", (int)textX + 150, (int)textY, 20, ctx->textSoft);
        textY += spacingY;

        DrawText("- System Lock:", (int)textX, (int)textY, 20, ctx->accentGold);
        DrawText("Enforces breaks by temporarily locking keyboard and mouse.", (int)textX + 160, (int)textY, 20, ctx->textSoft);
        textY += spacingY;

        DrawText("- Task Database:", (int)textX, (int)textY, 20, ctx->accentGold);
        DrawText("Save your sessions, titles, and notes to the SQLite history.", (int)textX + 185, (int)textY, 20, ctx->textSoft);
        textY += spacingY + 15.0f;

        DrawRectangleRounded((Rectangle) { textX - 10, textY - 10, ctx->panelW - 100, 100 }, UI_RADIUS, UI_SEGS, ctx->bgPanel);
        DrawText("Emergency Exit:", (int)textX + 10, (int)textY + 10, 24, (Color) { 240, 100, 100, 255 });
        DrawText("If you get stuck during a locked break, press:", (int)textX + 10, (int)textY + 45, 20, WHITE);
        DrawText("CTRL + ALT + SHIFT + Q", (int)textX + 480, (int)textY + 45, 22, ctx->accentGold);

        Rectangle backBtn = { (float)panelX + ((float)ctx->panelW - 240.0f) / 2.0f, (float)panelY + (float)ctx->panelH - 90.0f, 240.0f, 50.0f };
        if (DrawButton(backBtn, "BACK TO SETUP", ctx->btnNormal, ctx->btnHover, WHITE)) ctx->showHelpScreen = false;

        HandleTransitions(ctx);
        return;
    }

    const char* title = "FRIENDLY COMPUTER CAT";
    DrawText(title, panelX + (ctx->panelW - MeasureText(title, 32)) / 2, panelY + 40, 32, ctx->accentGreen);
    DrawLine(panelX + 50, panelY + 90, panelX + ctx->panelW - 50, panelY + 90, ctx->btnNormal);

    Rectangle helpBtn = { (float)panelX + (float)ctx->panelW - 100.0f, (float)panelY + 30.0f, 40.0f, 40.0f };
    Rectangle closeBtn = { (float)panelX + (float)ctx->panelW - 50.0f, (float)panelY + 30.0f, 40.0f, 40.0f };
    if (DrawButton(helpBtn, "?", ctx->bgPanel, ctx->btnHover, ctx->accentGold)) ctx->showHelpScreen = true;
    if (DrawButton(closeBtn, "X", ctx->bgPanel, (Color) { 200, 60, 60, 255 }, WHITE)) {
        ctx->isRunning = false;
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    Color cardBg = (Color){ 38, 41, 48, 255 };
    Color sepLine = (Color){ 55, 60, 70, 255 };

    float cardX = (float)panelX + 40.0f;
    float cardW = (float)ctx->panelW - 80.0f;

    float btnW = 40.0f, boxW = 70.0f, spc = 10.0f;
    float tGrpW = btnW + spc + boxW + spc + btnW;
    float secGroupX = cardX + cardW - tGrpW - 30.0f;
    float minGroupX = secGroupX - tGrpW - 20.0f;

    float inputW = (cardW < 600.0f) ? (cardW - 180.0f) : (cardW - 350.0f);

    float remainingHeight = (float)ctx->panelH - 200.0f;
    float c1H = remainingHeight * 0.25f;
    float c2H = remainingHeight * 0.45f;
    float c3H = remainingHeight * 0.25f;
    float cardGaps = remainingHeight * 0.05f / 2.0f;

    float c1Y = (float)panelY + 110.0f;
    DrawRectangleRounded((Rectangle) { cardX, c1Y, cardW, c1H }, 0.1f, UI_SEGS, cardBg);

    float c1RowStep = c1H / 2.0f;
    float itemY = c1Y + (c1RowStep - 45.0f) / 2.0f;

    DrawText("Profile Name", (int)cardX + 25, (int)itemY + 12, 20, ctx->textSoft);
    Rectangle nameRect = { cardX + cardW - inputW - 25.0f, itemY, inputW, 45.0f };
    if (CheckCollisionPointRec(mouse, nameRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 0;
    DrawRectangleRounded(nameRect, UI_RADIUS, UI_SEGS, ctx->activeField == 0 ? ctx->activeBox : ctx->bgPanel);
    DrawText(ctx->userName, (int)(nameRect.x + 15.0f), (int)(nameRect.y + 12.0f), 20, ctx->activeField == 0 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 0 && showCursor) DrawText("|", (int)(nameRect.x + 15.0f) + MeasureText(ctx->userName, 20) + 2, (int)(nameRect.y + 12.0f), 20, ctx->accentGold);

    DrawLine((int)cardX + 20, (int)(c1Y + c1RowStep), (int)(cardX + cardW - 20.0f), (int)(c1Y + c1RowStep), sepLine);

    itemY = c1Y + c1RowStep + (c1RowStep - 45.0f) / 2.0f;
    DrawText("Reminder Note", (int)cardX + 25, (int)itemY + 12, 20, ctx->textSoft);
    Rectangle remRect = { cardX + cardW - inputW - 25.0f, itemY, inputW, 45.0f };
    if (CheckCollisionPointRec(mouse, remRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 1;
    DrawRectangleRounded(remRect, UI_RADIUS, UI_SEGS, ctx->activeField == 1 ? ctx->activeBox : ctx->bgPanel);
    DrawText(ctx->reminderMessage, (int)(remRect.x + 15.0f), (int)(remRect.y + 12.0f), 20, ctx->activeField == 1 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 1 && showCursor) DrawText("|", (int)(remRect.x + 15.0f) + MeasureText(ctx->reminderMessage, 20) + 2, (int)(remRect.y + 12.0f), 20, ctx->accentGold);

    float c2Y = c1Y + c1H + cardGaps;
    DrawRectangleRounded((Rectangle) { cardX, c2Y, cardW, c2H }, 0.1f, UI_SEGS, cardBg);

    float topOffset = 35.0f;
    DrawText("MIN", (int)(minGroupX + (tGrpW - (float)MeasureText("MIN", 18)) / 2.0f), (int)(c2Y + 15.0f), 18, Fade(ctx->textSoft, 0.6f));
    DrawText("SEC", (int)(secGroupX + (tGrpW - (float)MeasureText("SEC", 18)) / 2.0f), (int)(c2Y + 15.0f), 18, Fade(ctx->textSoft, 0.6f));

    float rowStep = (c2H - topOffset) / 3.0f;

    itemY = c2Y + topOffset + (rowStep - 45.0f) / 2.0f;
    DrawText("Focus Duration", (int)cardX + 25, (int)itemY + 12, 20, ctx->textSoft);
    Rectangle wMinMinus = { minGroupX, itemY, btnW, 45.0f }, wMinRect = { minGroupX + btnW + spc, itemY, boxW, 45.0f }, wMinPlus = { wMinRect.x + boxW + spc, itemY, btnW, 45.0f };
    if (CheckCollisionPointRec(mouse, wMinRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 2;
    DrawRectangleRounded(wMinRect, UI_RADIUS, UI_SEGS, ctx->activeField == 2 ? ctx->activeBox : ctx->bgPanel);
    DrawText(TextFormat("%02d", ctx->workMin), (int)(wMinRect.x + 22.0f), (int)(wMinRect.y + 12.0f), 22, ctx->activeField == 2 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 2 && showCursor) DrawText("|", (int)(wMinRect.x + 22.0f) + MeasureText("00", 22), (int)(wMinRect.y + 12.0f), 22, ctx->accentGold);
    if (DrawButtonRepeat(wMinMinus, "-", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->workMin = (ctx->workMin > 0) ? ctx->workMin - 1 : 0;
    if (DrawButtonRepeat(wMinPlus, "+", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->workMin++;

    Rectangle wSecMinus = { secGroupX, itemY, btnW, 45.0f }, wSecRect = { secGroupX + btnW + spc, itemY, boxW, 45.0f }, wSecPlus = { wSecRect.x + boxW + spc, itemY, btnW, 45.0f };
    if (CheckCollisionPointRec(mouse, wSecRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 3;
    DrawRectangleRounded(wSecRect, UI_RADIUS, UI_SEGS, ctx->activeField == 3 ? ctx->activeBox : ctx->bgPanel);
    DrawText(TextFormat("%02d", ctx->workSec), (int)(wSecRect.x + 22.0f), (int)(wSecRect.y + 12.0f), 22, ctx->activeField == 3 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 3 && showCursor) DrawText("|", (int)(wSecRect.x + 22.0f) + MeasureText("00", 22), (int)(wSecRect.y + 12.0f), 22, ctx->accentGold);
    if (DrawButtonRepeat(wSecMinus, "-", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->workSec = (ctx->workSec > 0) ? ctx->workSec - 1 : 59;
    if (DrawButtonRepeat(wSecPlus, "+", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->workSec = (ctx->workSec < 59) ? ctx->workSec + 1 : 0;

    DrawLine((int)cardX + 20, (int)(c2Y + topOffset + rowStep), (int)(cardX + cardW - 20.0f), (int)(c2Y + topOffset + rowStep), sepLine);

    itemY = c2Y + topOffset + rowStep + (rowStep - 45.0f) / 2.0f;
    DrawText("Break Duration", (int)cardX + 25, (int)itemY + 12, 20, ctx->textSoft);
    Rectangle bMinMinus = { minGroupX, itemY, btnW, 45.0f }, bMinRect = { minGroupX + btnW + spc, itemY, boxW, 45.0f }, bMinPlus = { bMinRect.x + boxW + spc, itemY, btnW, 45.0f };
    if (CheckCollisionPointRec(mouse, bMinRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 4;
    DrawRectangleRounded(bMinRect, UI_RADIUS, UI_SEGS, ctx->activeField == 4 ? ctx->activeBox : ctx->bgPanel);
    DrawText(TextFormat("%02d", ctx->breakMin), (int)(bMinRect.x + 22.0f), (int)(bMinRect.y + 12.0f), 22, ctx->activeField == 4 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 4 && showCursor) DrawText("|", (int)(bMinRect.x + 22.0f) + MeasureText("00", 22), (int)(bMinRect.y + 12.0f), 22, ctx->accentGold);
    if (DrawButtonRepeat(bMinMinus, "-", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->breakMin = (ctx->breakMin > 0) ? ctx->breakMin - 1 : 0;
    if (DrawButtonRepeat(bMinPlus, "+", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->breakMin++;

    Rectangle bSecMinus = { secGroupX, itemY, btnW, 45.0f }, bSecRect = { secGroupX + btnW + spc, itemY, boxW, 45.0f }, bSecPlus = { bSecRect.x + boxW + spc, itemY, btnW, 45.0f };
    if (CheckCollisionPointRec(mouse, bSecRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 5;
    DrawRectangleRounded(bSecRect, UI_RADIUS, UI_SEGS, ctx->activeField == 5 ? ctx->activeBox : ctx->bgPanel);
    DrawText(TextFormat("%02d", ctx->breakSec), (int)(bSecRect.x + 22.0f), (int)(bSecRect.y + 12.0f), 22, ctx->activeField == 5 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 5 && showCursor) DrawText("|", (int)(bSecRect.x + 22.0f) + MeasureText("00", 22), (int)(bSecRect.y + 12.0f), 22, ctx->accentGold);
    if (DrawButtonRepeat(bSecMinus, "-", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->breakSec = (ctx->breakSec > 0) ? ctx->breakSec - 1 : 59;
    if (DrawButtonRepeat(bSecPlus, "+", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->breakSec = (ctx->breakSec < 59) ? ctx->breakSec + 1 : 0;

    DrawLine((int)cardX + 20, (int)(c2Y + topOffset + rowStep * 2.0f), (int)(cardX + cardW - 20.0f), (int)(c2Y + topOffset + rowStep * 2.0f), sepLine);

    itemY = c2Y + topOffset + rowStep * 2.0f + (rowStep - 45.0f) / 2.0f;
    DrawText("Show Warning Before", (int)cardX + 25, (int)itemY + 12, 20, ctx->textSoft);
    Rectangle warnMinus = { secGroupX, itemY, btnW, 45.0f }, warnRect = { secGroupX + btnW + spc, itemY, boxW, 45.0f }, warnPlus = { warnRect.x + boxW + spc, itemY, btnW, 45.0f };
    if (CheckCollisionPointRec(mouse, warnRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 6;
    DrawRectangleRounded(warnRect, UI_RADIUS, UI_SEGS, ctx->activeField == 6 ? ctx->activeBox : ctx->bgPanel);
    DrawText(TextFormat("%02d", ctx->warningSec), (int)(warnRect.x + 22.0f), (int)(warnRect.y + 12.0f), 22, ctx->activeField == 6 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 6 && showCursor) DrawText("|", (int)(warnRect.x + 22.0f) + MeasureText("00", 22), (int)(warnRect.y + 12.0f), 22, ctx->accentGold);
    if (DrawButtonRepeat(warnMinus, "-", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->warningSec = (ctx->warningSec > 1) ? ctx->warningSec - 1 : 1;
    if (DrawButtonRepeat(warnPlus, "+", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->warningSec = (ctx->warningSec < 60) ? ctx->warningSec + 1 : 60;

    float c3Y = c2Y + c2H + cardGaps;
    DrawRectangleRounded((Rectangle) { cardX, c3Y, cardW, c3H }, 0.1f, UI_SEGS, cardBg);

    float c3RowStep = c3H / 2.0f;

    itemY = c3Y + (c3RowStep - 45.0f) / 2.0f;
    DrawText("Total Cycles", (int)cardX + 25, (int)itemY + 12, 20, ctx->textSoft);
    Rectangle cycMinus = { secGroupX, itemY, btnW, 45.0f }, cycRect = { secGroupX + btnW + spc, itemY, boxW, 45.0f }, cycPlus = { cycRect.x + boxW + spc, itemY, btnW, 45.0f };
    if (CheckCollisionPointRec(mouse, cycRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 7;
    DrawRectangleRounded(cycRect, UI_RADIUS, UI_SEGS, ctx->activeField == 7 ? ctx->activeBox : ctx->bgPanel);
    DrawText(TextFormat("%02d", ctx->totalCycles), (int)(cycRect.x + 22.0f), (int)(cycRect.y + 12.0f), 22, ctx->activeField == 7 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 7 && showCursor) DrawText("|", (int)(cycRect.x + 22.0f) + MeasureText("00", 22), (int)(cycRect.y + 12.0f), 22, ctx->accentGold);
    if (DrawButtonRepeat(cycMinus, "-", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->totalCycles = (ctx->totalCycles > 1) ? ctx->totalCycles - 1 : 1;
    if (DrawButtonRepeat(cycPlus, "+", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->totalCycles++;

    DrawLine((int)cardX + 20, (int)(c3Y + c3RowStep), (int)(cardX + cardW - 20.0f), (int)(c3Y + c3RowStep), sepLine);

    itemY = c3Y + c3RowStep + (c3RowStep - 45.0f) / 2.0f;
    DrawText("Alarm Volume", (int)cardX + 25, (int)itemY + 12, 20, ctx->textSoft);
    float volBoxW = 90.0f;
    float volGroupX = cardX + cardW - (btnW + spc + volBoxW + spc + btnW) - 30.0f;
    Rectangle volMinus = { volGroupX, itemY, btnW, 45.0f }, volRect = { volGroupX + btnW + spc, itemY, volBoxW, 45.0f }, volPlus = { volRect.x + volBoxW + spc, itemY, btnW, 45.0f };
    if (CheckCollisionPointRec(mouse, volRect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ctx->activeField = 8;
    DrawRectangleRounded(volRect, UI_RADIUS, UI_SEGS, ctx->activeField == 8 ? ctx->activeBox : ctx->bgPanel);
    DrawText(TextFormat("%03d %%", ctx->volumeLevel), (int)(volRect.x + 18.0f), (int)(volRect.y + 12.0f), 22, ctx->activeField == 8 ? ctx->accentGold : ctx->textSoft);
    if (ctx->activeField == 8 && showCursor) DrawText("|", (int)(volRect.x + 18.0f) + MeasureText("000 %", 22), (int)(volRect.y + 12.0f), 22, ctx->accentGold);
    if (DrawButtonRepeat(volMinus, "-", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->volumeLevel = (ctx->volumeLevel > 0) ? ctx->volumeLevel - 10 : 0;
    if (DrawButtonRepeat(volPlus, "+", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) ctx->volumeLevel = (ctx->volumeLevel < 100) ? ctx->volumeLevel + 10 : 100;

    float btnY = (float)panelY + (float)ctx->panelH - 80.0f;

    Rectangle dbBtn = { (float)panelX + 40.0f, btnY, 180.0f, 50.0f };
    if (DrawButton(dbBtn, "SCHEDULES", ctx->bgPanel, ctx->btnHover, ctx->textSoft)) {
        RefreshScheduleList(ctx);
        ctx->scheduleScrollY = 0;
        ctx->selectedScheduleId = -1;
        ctx->currentState = STATE_SCHEDULE_DB;
    }

    Rectangle startBtn = { (float)panelX + ((float)ctx->panelW - 320.0f) / 2.0f, btnY, 320.0f, 50.0f };

    if (DrawButton(startBtn, "START FOCUS", ctx->accentGreen, ctx->textSoft, ctx->bgDark)) {
        memset(ctx->promptTitle, 0, sizeof(ctx->promptTitle));
        memset(ctx->promptInfo, 0, sizeof(ctx->promptInfo));
        ctx->promptActiveField = 0;
        ctx->currentState = STATE_PROMPT_DB;
    }

    HandleTransitions(ctx);
}