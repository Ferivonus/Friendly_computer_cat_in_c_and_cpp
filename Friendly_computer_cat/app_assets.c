#include "app.h"
#include <pthread.h> 

static pthread_t assetThread;
static bool isThreadStarted = false;
static volatile bool isThreadFinished = false;

void* AssetLoadingWorker(void* arg) {
    AppContext* ctx = (AppContext*)arg;

    if (FileExists(ctx->assetWorkGif)) {
        ctx->imAnim1 = LoadImageAnim(ctx->assetWorkGif, &ctx->animFrames1);
    }
    else ctx->imAnim1.data = NULL;
    ctx->loadStep = 2;

    if (FileExists(ctx->assetBreakGif)) {
        ctx->imAnim2 = LoadImageAnim(ctx->assetBreakGif, &ctx->animFrames2);
    }
    else ctx->imAnim2.data = NULL;
    ctx->loadStep = 3;

    if (FileExists(ctx->assetInfoGif)) {
        ctx->imAnim3 = LoadImageAnim(ctx->assetInfoGif, &ctx->animFrames3);
    }
    else ctx->imAnim3.data = NULL;
    ctx->loadStep = 4;

    if (FileExists(ctx->assetGoodbyeGif)) {
        ctx->imAnim4 = LoadImageAnim(ctx->assetGoodbyeGif, &ctx->animFrames4);
    }
    else ctx->imAnim4.data = NULL;

    isThreadFinished = true;
    return NULL;
}

bool UpdateAssetLoading(AppContext* ctx) {
    if (ctx == NULL) return true;

    if (!isThreadStarted) {
        if (FileExists(ctx->assetAlarmSound)) {
            ctx->alarmSound = LoadSound(ctx->assetAlarmSound);
        }
        else {
            printf("WARNING: Audio file not found (%s). Defaulting to silent mode.\n", ctx->assetAlarmSound);
            ctx->alarmSound.stream.buffer = NULL;
        }
        ctx->loadStep = 1;

        pthread_create(&assetThread, NULL, AssetLoadingWorker, (void*)ctx);
        isThreadStarted = true;

        return false; 
    }

    if (isThreadFinished) {

        if (ctx->imAnim1.data != NULL) ctx->texAnim1 = LoadTextureFromImage(ctx->imAnim1);
        if (ctx->imAnim2.data != NULL) ctx->texAnim2 = LoadTextureFromImage(ctx->imAnim2);
        if (ctx->imAnim3.data != NULL) ctx->texAnim3 = LoadTextureFromImage(ctx->imAnim3);
        if (ctx->imAnim4.data != NULL) ctx->texAnim4 = LoadTextureFromImage(ctx->imAnim4);

        ctx->loadStep = 5; 

        pthread_join(assetThread, NULL);

        isThreadStarted = false;
        isThreadFinished = false;

        return true;
    }

    return false;
}

void UnloadAppAssets(AppContext* ctx) {
    if (ctx == NULL) return;

    if (ctx->alarmSound.stream.buffer != NULL) {
        UnloadSound(ctx->alarmSound);
    }
    if (ctx->imAnim1.data != NULL) {
        UnloadTexture(ctx->texAnim1);
        UnloadImage(ctx->imAnim1);
    }
    if (ctx->imAnim2.data != NULL) {
        UnloadTexture(ctx->texAnim2);
        UnloadImage(ctx->imAnim2);
    }
    if (ctx->imAnim3.data != NULL) {
        UnloadTexture(ctx->texAnim3);
        UnloadImage(ctx->imAnim3);
    }
    if (ctx->imAnim4.data != NULL) {
        UnloadTexture(ctx->texAnim4);
        UnloadImage(ctx->imAnim4);
    }
}