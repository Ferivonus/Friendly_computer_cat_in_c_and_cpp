#ifndef WORKSCHEDULE_H
#define WORKSCHEDULE_H

#include <string>
#include <vector>
#include <iostream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include "spdlog/spdlog.h"
#include "LinkedList.h"
#include <stdexcept>

using json = nlohmann::json;

class WorkSchedule {
public:
    WorkSchedule(const std::string database_name);

    struct WorkScheduleItem {
        int id = 0;
        std::string work_info;
        std::string work_title;
        int working_time_as_minute = 0;
        int how_many_turns_work = 0;
        int starting_working_time = 0;
        int did_time_goes_up = 0;
        std::string which_turs_time_goes_up;
        std::string starting_time;
    };

    int add_schedule(const WorkScheduleItem& schedule);

    void delete_work_done(int);

    LinkedList* list_work_done();

    void update_work(const WorkScheduleItem& updating_schedule);

    C_WorkScheduleItem get_work_info(int wanted_info_id);

private:
    void initialize_database();
    bool set_default_database();

    sqlite3* db = nullptr;
    std::string database_name;
};

#endif