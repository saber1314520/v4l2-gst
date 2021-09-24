#pragma once

#include <ctime>
#include <string>

namespace gpptest
{
    class timestamp
    {
    private:
        std::time_t rawtime;
        struct tm *ptime;

    public:
        timestamp();
        bool time_update();
        int get_info(char*info_type);
        std::string timestamp_to_string();//[%y-%m-%d %h:%m:%s]
        std::string date_to_string();//[%y-%m-%d]
        std::string time_to_string();//[%h:%m:%s]
    };
}
