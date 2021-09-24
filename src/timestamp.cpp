#include "timestamp.h"

namespace gpptest
{
    timestamp::timestamp()
    {
        if(!time_update())
        {
            rawtime=-1;
            ptime=nullptr;
        }
    }

    bool timestamp::time_update()
    {
        if(time(&rawtime)==-1)return false;
        ptime=localtime(&rawtime);
        return true;
    }

    std::string timestamp::timestamp_to_string()
    {
        if(rawtime==-1)return std::string("[0000-00-00 00:00:00]");
        char str[256];
        strftime(str,sizeof(str),"[%Y-%m-%d %H:%M:%S]",ptime);
        std::string time =str;
        return time;
    }

    std::string timestamp::date_to_string()
    {
        if(rawtime==-1)return std::string("[0000-00-00]");
        char str[256];
        strftime(str,sizeof(str),"[%Y-%m-%d]",ptime);
        std::string time =str;
        return time;
    }

    std::string timestamp::time_to_string()
    {
        if(rawtime==-1)return std::string("[00:00:00]");
        char str[256];
        strftime(str,sizeof(str),"[%H:%M:%S]",ptime);
        std::string time =str;
        return time;
    }
}