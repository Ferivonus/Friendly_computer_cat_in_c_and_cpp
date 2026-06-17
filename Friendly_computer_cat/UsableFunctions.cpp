#include "UsableFunctions.h"


 UsableFunctions::time_Struct UsableFunctions::Get_time_Function_as_time_struct(){


    struct tm newtime;
    char am_pm[] = "AM";
    __time64_t long_time;
    char timebuf[26];
    errno_t err;

    _time64(&long_time);
    err = _localtime64_s(&newtime, &long_time);
    if (err)
    {
        printf("Invalid argument to _localtime64_s.");
        exit(1);
    }
    if (newtime.tm_hour > 12)
        strcpy_s(am_pm, sizeof(am_pm), "PM");
    if (newtime.tm_hour > 12)
        newtime.tm_hour -= 12;
    if (newtime.tm_hour == 0)
        newtime.tm_hour = 12;

    // Convert to an ASCII representation.
    err = asctime_s(timebuf, 26, &newtime);
    if (err)
    {
        printf("Invalid argument to asctime_s.");
        exit(1);
    }
    printf("%.19s %s\n", timebuf, am_pm);

    UsableFunctions::time_Struct sended_time{
        *timebuf,
        *am_pm
    };

    return (sended_time);
}


 int_fast8_t UsableFunctions::random_number_generator_for_working_time(int_fast8_t top, int_fast8_t floor, bool is_minute_or_second) {

     if (top == floor) {
         return top;
     }

     if (top < floor) {
         std::swap(top, floor);
     }

     static std::random_device r;
     static std::default_random_engine e1(r());

     std::uniform_int_distribution<int> uniform_dist(+floor, +top);
     int_fast8_t random_value = uniform_dist(e1);

     if (is_minute_or_second) {
         spdlog::info("The chosen random minute is: {}", +random_value);
     }
     else {
         spdlog::info("The chosen random second is: {}", +random_value);
     }

     return random_value;
 }

