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

    typedef struct  {
        int id;
        std::string work_info;
        std::string work_title;
        int working_time_as_minute;
        int how_many_turns_work;
        int starting_working_time;
        int did_time_goes_up;
        std::string which_turs_time_goes_up;
        std::string starting_time;
    }WorkScheduleItem;
    
	
    int add_schedule(const WorkSchedule::WorkScheduleItem& schedule);

    void delete_work_done(int);

    LinkedList* list_work_done();
    

    void update_work(const WorkSchedule::WorkScheduleItem& updating_schedule);

    C_WorkScheduleItem get_work_info(int wanted_info_id);


private:
    void initialize_database();

    bool set_default_database();


    sqlite3* db;
    std::string database_name;
};

#endif // WORKSCHEDULE_H