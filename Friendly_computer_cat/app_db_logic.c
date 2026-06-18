#include "app.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    AppContext* ctx;
    C_WorkScheduleItem item;
    int operationType;
} DbTaskArgs;

void RefreshScheduleList(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL) return;

    if (ctx->currentScheduleList != NULL) {
        freeLinkedList(ctx->currentScheduleList);
        ctx->currentScheduleList = NULL;
    }

    ctx->currentScheduleList = list_work_done_wrapper(ctx->dbHandle);
}

static void* DbWorkerThread(void* arg) {
    DbTaskArgs* task = (DbTaskArgs*)arg;

    if (task->operationType == 1) {
        int newId = add_Work_to_database_wrapper(task->ctx->dbHandle, task->item);
        if (newId != -1) {
            task->ctx->selectedScheduleId = newId;
        }
    }
    else if (task->operationType == 2) {
        update_work_done_wrapper(task->ctx->dbHandle, task->item);
    }
    else if (task->operationType == 3) {
        delete_specific_work_schedule_wrapper(task->ctx->dbHandle, task->item.id);
        task->ctx->selectedScheduleId = -1;
    }

    RefreshScheduleList(task->ctx);

    if (task->item.work_title) free((void*)task->item.work_title);
    if (task->item.work_info) free((void*)task->item.work_info);

    free(task);
    return NULL;
}

static const char* copy_string_for_thread(const char* source) {
    if (source == NULL) return NULL;
    size_t len = strlen(source);
    char* copy = (char*)malloc(len + 1);
    if (copy != NULL) {
        strcpy_s(copy, len + 1, source);
    }
    return copy;
}

void HandleAddSchedule(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL) return;

    DbTaskArgs* args = (DbTaskArgs*)malloc(sizeof(DbTaskArgs));
    if (args == NULL) return;

    args->ctx = ctx;
    args->operationType = 1;

    args->item.id = 0;
    args->item.working_time_as_minute = ctx->workMin;
    args->item.how_many_turns_work = ctx->totalCycles;
    args->item.starting_working_time = ctx->workMin;
    args->item.did_time_goes_up = 0;

    const char* tTitle = (ctx->promptTitle[0] != '\0') ? ctx->promptTitle : "Untitled Task";
    const char* tInfo = (ctx->promptInfo[0] != '\0') ? ctx->promptInfo : "No description provided.";

    args->item.work_title = copy_string_for_thread(tTitle);
    args->item.work_info = copy_string_for_thread(tInfo);
    args->item.which_turs_time_goes_up = NULL;
    args->item.starting_time = NULL;

    pthread_t threadId;
    pthread_create(&threadId, NULL, DbWorkerThread, args);
    pthread_detach(threadId);
}

void HandleUpdateSchedule(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL || ctx->selectedScheduleId == -1) return;

    DbTaskArgs* args = (DbTaskArgs*)malloc(sizeof(DbTaskArgs));
    if (args == NULL) return;

    args->ctx = ctx;
    args->operationType = 2;

    args->item.id = ctx->selectedScheduleId;
    args->item.working_time_as_minute = ctx->workMin;
    args->item.how_many_turns_work = ctx->totalCycles;
    args->item.starting_working_time = ctx->workMin;
    args->item.did_time_goes_up = 0;

    const char* tTitle = (ctx->promptTitle[0] != '\0') ? ctx->promptTitle : "Untitled Task";
    const char* tInfo = (ctx->promptInfo[0] != '\0') ? ctx->promptInfo : "No description provided.";

    args->item.work_title = copy_string_for_thread(tTitle);
    args->item.work_info = copy_string_for_thread(tInfo);
    args->item.which_turs_time_goes_up = NULL;
    args->item.starting_time = NULL;

    pthread_t threadId;
    pthread_create(&threadId, NULL, DbWorkerThread, args);
    pthread_detach(threadId);
}

void HandleDeleteSchedule(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL || ctx->selectedScheduleId == -1) return;

    DbTaskArgs* args = (DbTaskArgs*)malloc(sizeof(DbTaskArgs));
    if (args == NULL) return;

    args->ctx = ctx;
    args->operationType = 3;

    args->item.id = ctx->selectedScheduleId;
    args->item.work_title = NULL;
    args->item.work_info = NULL;

    pthread_t threadId;
    pthread_create(&threadId, NULL, DbWorkerThread, args);
    pthread_detach(threadId);
}