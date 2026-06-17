#include "WorkSchedule_Wrapper.h"
#include "WorkSchedule.h"
#include "LinkedList.h" 

static WorkSchedule::WorkScheduleItem dataLoader(const C_WorkScheduleItem& c_data) {
    WorkSchedule::WorkScheduleItem cpp_data;
    cpp_data.id = c_data.id;
    cpp_data.work_info = c_data.work_info ? c_data.work_info : "";
    cpp_data.work_title = c_data.work_title ? c_data.work_title : "";
    cpp_data.working_time_as_minute = c_data.working_time_as_minute;
    cpp_data.how_many_turns_work = c_data.how_many_turns_work;
    cpp_data.starting_working_time = c_data.starting_working_time;
    cpp_data.did_time_goes_up = c_data.did_time_goes_up;
    cpp_data.which_turs_time_goes_up = c_data.which_turs_time_goes_up ? c_data.which_turs_time_goes_up : "";
    cpp_data.starting_time = c_data.starting_time ? c_data.starting_time : "";

    return cpp_data;
}

extern "C" {

    WorkScheduleHandle start_work_schedule_wrapper(const char* database_name) {
        if (database_name == nullptr) {
            return nullptr;
        }
        return new WorkSchedule(database_name);
    }

    void destroy_work_schedule_wrapper(WorkScheduleHandle handle) {
        if (handle != nullptr) {
            delete static_cast<WorkSchedule*>(handle);
        }
    }

    void delete_specific_work_schedule_wrapper(WorkScheduleHandle handle, int work_id) {
        if (handle != nullptr && work_id >= 0) {
            static_cast<WorkSchedule*>(handle)->delete_work_done(work_id);
        }
    }

    int add_Work_to_database_wrapper(WorkScheduleHandle handle, C_WorkScheduleItem c_data) {
        if (handle == nullptr) return -1; 

        WorkSchedule::WorkScheduleItem cpp_data = dataLoader(c_data);

        return static_cast<WorkSchedule*>(handle)->add_schedule(cpp_data);
    }

    void update_work_done_wrapper(WorkScheduleHandle handle, C_WorkScheduleItem c_data) {
        if (handle == nullptr || c_data.id < 0) return;

        WorkSchedule::WorkScheduleItem cpp_data = dataLoader(c_data);
        static_cast<WorkSchedule*>(handle)->update_work(cpp_data);
    }

    LinkedList* list_work_done_wrapper(WorkScheduleHandle handle) {
        if (handle == nullptr) {
            return nullptr;
        }

        WorkSchedule* schedule = static_cast<WorkSchedule*>(handle);
        return schedule->list_work_done();
    }

    C_WorkScheduleItem get_specific_work_schedule_wrapper(WorkScheduleHandle handle, int work_id) {
        if (handle == nullptr || work_id < 0) {
            C_WorkScheduleItem emptyItem = { 0 };
            emptyItem.id = -1;
            return emptyItem;
        }

        WorkSchedule* schedule = static_cast<WorkSchedule*>(handle);
        return schedule->get_work_info(work_id);
    }

}