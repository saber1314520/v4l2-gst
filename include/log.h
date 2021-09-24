#pragma once
#include <fstream>
#include <string>

namespace gpptest
{
    enum class logtype
    {
        WARNING, //warning information,usually customized
        ERROR,   //gstreamer error message
        INFO,    //notification of the app usually gstreamer message except @WARING and @ERROR and device connection information
        DETAIL,  //details for the above
    };

    class log
    {
    private:
        std::string m_filename;
        std::string m_filepath; //filepath without filename,default ../log/
        std::fstream m_log;
        logtype m_type;

    private:
        bool open(char *file);

    public:
        log() : m_filepath("../log/"), m_type(logtype::INFO) { printf("Log is created,please open a file for following oprations!\n"); }
        bool open(std::string filename);
        void close();
        bool settype(logtype type);
        bool setpath(char *path);
        bool isopen();
        bool isthisfile(std::string filename);

        template <class T>
        log &writelog(T text);

        template <typename T>
        log &operator<<(T text);
    };
}