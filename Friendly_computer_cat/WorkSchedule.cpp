#include "WorkSchedule.h"
#include "LinkedList.h"

WorkSchedule::WorkSchedule(const std::string database_name) {
    spdlog::info("Database part has started with the database name of  " + database_name);
    this->database_name = database_name;
    initialize_database();
}

void WorkSchedule::initialize_database() {
    spdlog::info("Starting database initialization process...");

    try {
        spdlog::info("Attempting to open or create dynamic database...");

        std::string full_db_name = database_name + ".db";

        int result = sqlite3_open(full_db_name.c_str(), &db);

        if (result != SQLITE_OK) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Open Error: " + error_msg);
        }

        spdlog::info("Database opened successfully. Proceeding to check schemas...");

        set_default_database();

        spdlog::info("Database initialization process completed perfectly.");
    }
    catch (const std::exception& e) {
        spdlog::critical("Exception caught during database initialization: {}", e.what());
        spdlog::critical("Module cannot operate due to database failure.");

        if (db) {
            spdlog::info("Closing database connection due to initialization error...");
            sqlite3_close(db);
            db = nullptr;
        }

        throw;
    }
}

bool WorkSchedule::set_default_database() {
    spdlog::info("Checking if 'WorkSchedule' table exists in the database...");

    try {
        bool table_exists = false;
        const char* check_sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='WorkSchedule';";
        sqlite3_stmt* stmt;

        int prepare_rc = sqlite3_prepare_v2(db, check_sql, -1, &stmt, nullptr);
        if (prepare_rc != SQLITE_OK) {
            std::string error_str = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Prepare Error during check: " + error_str);
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            table_exists = true;
        }
        sqlite3_finalize(stmt);

        if (table_exists) {
            spdlog::info("Table 'WorkSchedule' already exists. Skipping creation process.");
            return true;
        }

        spdlog::info("Table 'WorkSchedule' not found. Proceeding to create it...");

        char* errMsg = nullptr;
        const char* create_sql = "CREATE TABLE WorkSchedule("
            "id                         INTEGER PRIMARY KEY NOT NULL, "
            "Work_info                  TEXT NOT NULL, "
            "Work_title                 TEXT NOT NULL, "
            "Working_time_as_minute     INTEGER NOT NULL, "
            "How_Many_Turns_work        INTEGER NOT NULL, "
            "Starting_working_time      INTEGER NOT NULL, "
            "Did_time_goes_up           INTEGER CHECK (Did_time_goes_up IN (0, 2)), "
            "which_turs_time_goes_up    CHARACTER(20) DEFAULT NULL, "
            "Starting_time              DATETIME DEFAULT CURRENT_TIMESTAMP"
            ");";

        spdlog::info("Executing CREATE TABLE query for WorkSchedule...");
        int rc = sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg);

        if (rc != SQLITE_OK) {
            std::string error_str = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("SQLite Exec Error: " + error_str);
        }

        spdlog::info("Table 'WorkSchedule' successfully created and ready to use.");
        return true;
    }
    catch (const std::exception& e) {
        spdlog::critical("Exception caught while creating/verifying tables: {}", e.what());
        throw;
    }
}

int WorkSchedule::add_schedule(const WorkSchedule::WorkScheduleItem& schedule) {
    sqlite3_stmt* stmt = nullptr;

    const char* insert_sql = "INSERT INTO WorkSchedule ("
        "Work_info, Work_title, Working_time_as_minute, "
        "How_Many_Turns_work, Starting_working_time, "
        "Did_time_goes_up, which_turs_time_goes_up"
        ") VALUES (?, ?, ?, ?, ?, ?, ?);";

    try {
        int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Prepare Error: " + error_msg);
        }

        sqlite3_bind_text(stmt, 1, schedule.work_info.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, schedule.work_title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, schedule.working_time_as_minute);
        sqlite3_bind_int(stmt, 4, schedule.how_many_turns_work);
        sqlite3_bind_int(stmt, 5, schedule.starting_working_time);
        sqlite3_bind_int(stmt, 6, schedule.did_time_goes_up);

        if (schedule.which_turs_time_goes_up.empty()) {
            sqlite3_bind_null(stmt, 7);
        }
        else {
            sqlite3_bind_text(stmt, 7, schedule.which_turs_time_goes_up.c_str(), -1, SQLITE_TRANSIENT);
        }

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Execution Error: " + error_msg);
        }

        int generated_id = static_cast<int>(sqlite3_last_insert_rowid(db));

        sqlite3_finalize(stmt);
        spdlog::info("New work schedule item inserted successfully with ID: {}", generated_id);

        return generated_id;
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to insert schedule: {}", e.what());

        if (stmt) {
            sqlite3_finalize(stmt);
        }
        throw;
    }
}
void WorkSchedule::delete_work_done(int work_id_will_be_deleted) {
    sqlite3_stmt* stmt = nullptr;
    const char* delete_sql = "DELETE FROM WorkSchedule WHERE id = ?";

    try {
        int rc = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Prepare Error: " + error_msg);
        }

        sqlite3_bind_int(stmt, 1, work_id_will_be_deleted);

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Execution Error: " + error_msg);
        }

        sqlite3_finalize(stmt);
        spdlog::info("Work schedule item deleted successfully.");
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to delete schedule: {}", e.what());

        if (stmt) {
            sqlite3_finalize(stmt);
        }

        throw;
    }
}

static const char* safe_strdup(const unsigned char* sqlite_text) {
    if (sqlite_text == nullptr) {
        return nullptr;
    }

    size_t len = std::strlen(reinterpret_cast<const char*>(sqlite_text));
    char* copy = static_cast<char*>(std::malloc(len + 1));

    if (copy == nullptr) {
        return nullptr;
    }

    std::memcpy(copy, sqlite_text, len + 1);

    return copy;
}

LinkedList* WorkSchedule::list_work_done() {
    LinkedList* my_list = createLinkedList();

    sqlite3_stmt* stmt = nullptr;
    const char* select_sql = "SELECT * FROM WorkSchedule;";

    try {
        int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Prepare Error: " + error_msg);
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            C_WorkScheduleItem item{};

            item.id = sqlite3_column_int(stmt, 0);

            item.work_info = safe_strdup(sqlite3_column_text(stmt, 1));
            item.work_title = safe_strdup(sqlite3_column_text(stmt, 2));

            item.working_time_as_minute = sqlite3_column_int(stmt, 3);
            item.how_many_turns_work = sqlite3_column_int(stmt, 4);
            item.starting_working_time = sqlite3_column_int(stmt, 5);
            item.did_time_goes_up = sqlite3_column_int(stmt, 6);

            item.which_turs_time_goes_up = safe_strdup(sqlite3_column_text(stmt, 7));
            item.starting_time = safe_strdup(sqlite3_column_text(stmt, 8));

            addLast(my_list, item);
        }

        sqlite3_finalize(stmt);
        spdlog::info("Work schedule items listed successfully. Total items: {}", my_list->length);

        return my_list;
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to list schedule: {}", e.what());

        if (stmt) {
            sqlite3_finalize(stmt);
        }

        if (my_list) {
            freeLinkedList(my_list);
        }

        throw;
    }
}

void WorkSchedule::update_work(const WorkSchedule::WorkScheduleItem& updating_schedule) {
    sqlite3_stmt* stmt = nullptr;

    const char* update_sql = "UPDATE WorkSchedule SET "
        "Work_info = ?, "
        "Work_title = ?, "
        "Working_time_as_minute = ?, "
        "How_Many_Turns_work = ?, "
        "Starting_working_time = ?, "
        "Did_time_goes_up = ?, "
        "which_turs_time_goes_up = ? "
        "WHERE id = ?;";

    try {
        int rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Prepare Error (Update): " + error_msg);
        }

        sqlite3_bind_text(stmt, 1, updating_schedule.work_info.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, updating_schedule.work_title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, updating_schedule.working_time_as_minute);
        sqlite3_bind_int(stmt, 4, updating_schedule.how_many_turns_work);
        sqlite3_bind_int(stmt, 5, updating_schedule.starting_working_time);
        sqlite3_bind_int(stmt, 6, updating_schedule.did_time_goes_up);

        if (updating_schedule.which_turs_time_goes_up.empty()) {
            sqlite3_bind_null(stmt, 7);
        }
        else {
            sqlite3_bind_text(stmt, 7, updating_schedule.which_turs_time_goes_up.c_str(), -1, SQLITE_TRANSIENT);
        }

        sqlite3_bind_int(stmt, 8, updating_schedule.id);

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Execution Error (Update): " + error_msg);
        }

        int changes = sqlite3_changes(db);
        if (changes == 0) {
            spdlog::warn("Update schedule called, but no rows were modified. Check if ID {} exists.", updating_schedule.id);
        }
        else {
            spdlog::info("Work schedule item (ID: {}) updated successfully.", updating_schedule.id);
        }

        sqlite3_finalize(stmt);
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to update schedule: {}", e.what());

        if (stmt) {
            sqlite3_finalize(stmt);
        }
        throw;
    }
}

C_WorkScheduleItem WorkSchedule::get_work_info(int wanted_info_id) {
    sqlite3_stmt* stmt = nullptr;

    const char* select_sql = "SELECT * FROM WorkSchedule WHERE id = ?;";

    C_WorkScheduleItem item{};
    item.id = -1; 

    try {
        int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);

        if (rc != SQLITE_OK) {
            std::string error_msg = sqlite3_errmsg(db);
            throw std::runtime_error("SQLite Prepare Error: " + error_msg);
        }

        sqlite3_bind_int(stmt, 1, wanted_info_id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            item.id = sqlite3_column_int(stmt, 0);
            item.work_info = safe_strdup(sqlite3_column_text(stmt, 1));
            item.work_title = safe_strdup(sqlite3_column_text(stmt, 2));
            item.working_time_as_minute = sqlite3_column_int(stmt, 3);
            item.how_many_turns_work = sqlite3_column_int(stmt, 4);
            item.starting_working_time = sqlite3_column_int(stmt, 5);
            item.did_time_goes_up = sqlite3_column_int(stmt, 6);
            item.which_turs_time_goes_up = safe_strdup(sqlite3_column_text(stmt, 7));
            item.starting_time = safe_strdup(sqlite3_column_text(stmt, 8));
        }

        sqlite3_finalize(stmt);

        spdlog::info("Work schedule item id {} ready to be sent correctly.", wanted_info_id);

        return item;
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to fetch schedule info: {}", e.what());

        if (stmt) {
            sqlite3_finalize(stmt);
        }
        throw;
    }
}