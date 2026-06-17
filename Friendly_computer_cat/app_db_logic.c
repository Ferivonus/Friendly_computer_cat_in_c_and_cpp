#include "app.h"

void RefreshScheduleList(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL) return;

    if (ctx->currentScheduleList != NULL) {
        freeLinkedList(ctx->currentScheduleList);
        ctx->currentScheduleList = NULL;
    }

    ctx->currentScheduleList = list_work_done_wrapper(ctx->dbHandle);
}

void HandleAddSchedule(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL) return;

    C_WorkScheduleItem newItem = { 0 };
    newItem.id = 0; 

    newItem.work_title = (ctx->promptTitle[0] != '\0') ? ctx->promptTitle : "Untitled Task";
    newItem.work_info = (ctx->promptInfo[0] != '\0') ? ctx->promptInfo : "No description provided.";

    newItem.working_time_as_minute = ctx->workMin;
    newItem.how_many_turns_work = ctx->totalCycles;
    newItem.starting_working_time = ctx->workMin;
    newItem.did_time_goes_up = 0;

    int newId = add_Work_to_database_wrapper(ctx->dbHandle, newItem);

    if (newId != -1) {
        ctx->selectedScheduleId = newId;
    }
}

void HandleUpdateSchedule(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL || ctx->selectedScheduleId == -1) return;

    C_WorkScheduleItem updateItem = { 0 };
    updateItem.id = ctx->selectedScheduleId; 

    updateItem.work_title = (ctx->promptTitle[0] != '\0') ? ctx->promptTitle : "Untitled Task";
    updateItem.work_info = (ctx->promptInfo[0] != '\0') ? ctx->promptInfo : "No description provided.";

    updateItem.working_time_as_minute = ctx->workMin;
    updateItem.how_many_turns_work = ctx->totalCycles;
    updateItem.starting_working_time = ctx->workMin;
    updateItem.did_time_goes_up = 0;

    update_work_done_wrapper(ctx->dbHandle, updateItem);
}

void HandleDeleteSchedule(AppContext* ctx) {
    if (ctx == NULL || ctx->dbHandle == NULL) return;
    if (ctx->selectedScheduleId == -1) return;

    delete_specific_work_schedule_wrapper(ctx->dbHandle, ctx->selectedScheduleId);

    ctx->selectedScheduleId = -1;
    RefreshScheduleList(ctx);
}