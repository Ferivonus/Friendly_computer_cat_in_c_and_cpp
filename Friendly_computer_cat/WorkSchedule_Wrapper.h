#ifndef WORKSCHEDULE_WRAPPER_H
#define WORKSCHEDULE_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

    typedef void* WorkScheduleHandle;

    typedef struct {
        int id;
        const char* work_info;
        const char* work_title;
        int working_time_as_minute;
        int how_many_turns_work;
        int starting_working_time;
        int did_time_goes_up;
        const char* which_turs_time_goes_up;
        const char* starting_time;
    } C_WorkScheduleItem;

    typedef struct LinkedList LinkedList;

    WorkScheduleHandle start_work_schedule_wrapper(const char* database_name);

    void destroy_work_schedule_wrapper(WorkScheduleHandle handle);

    void delete_specific_work_schedule_wrapper(WorkScheduleHandle handle, int work_id);

    int add_Work_to_database_wrapper(WorkScheduleHandle handle, C_WorkScheduleItem data);

    void update_work_done_wrapper(WorkScheduleHandle handle, C_WorkScheduleItem updating_data);

    LinkedList* list_work_done_wrapper(WorkScheduleHandle handle);

    C_WorkScheduleItem get_specific_work_schedule_wrapper(WorkScheduleHandle handle, int work_id);

#ifdef __cplusplus
}
#endif

#endif // WORKSCHEDULE_WRAPPER_H