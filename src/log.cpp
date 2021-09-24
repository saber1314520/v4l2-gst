#include "log.h"

#include <sys/time.h>

namespace gpptest
{
    void log::close()
    {
        if (m_log.is_open())
        {
            m_log.close();
        }
    }

    bool log::open(std::string filename)
    {
        m_filename = filename;
        m_log.open(filename.c_str(), std::ios_base::out|std::ios_base::app);
        if (m_log.is_open())
        {
            return true;
        }
        printf("Failed to open file %s\n", (m_filepath + filename).c_str());
        return false;
    }

    bool log::isopen()
    {
        return m_log.is_open();
    }

    bool log::settype(logtype type)
    {
        switch (type)
        {
        case logtype::WARNING:
        case logtype::ERROR:
        case logtype::INFO:
        case logtype::DETAIL:
            m_type = type;
            return true;
            break;
        default:
            return false;
            break;
        }
    }

    bool log::setpath(char *path)
    {

        m_filepath = path;
        return true;
    }

    bool log::isthisfile(std::string filename)
    {
        return m_filename == filename;
    }

    template <class T>
    log &log::writelog(T text)
    {
        // switch (m_type)
        // {
        // case logtype::WARNING:
        //     break;
        // case logtype::ERROR:
        //     break;
        // case logtype::INFO:
        //     break;
        // case logtype::DETAIL:
        //     break;

        // default:
        //     break;
        // }
        m_log << text;
        return *this;
    }

    template <typename T>
    log &log::operator<<(T text)
    {
        m_log << text;
        return *this;
    }

    template log &log::operator<<<const char *>(const char *text); //explicit instantiation

    template log &log::operator<<<char *>(char *text);

    template log &log::operator<<<std::string>(std::string text);

    // template <>
    // log &log::operator<< (const char *text)//explicit specialization
    // {
    //     m_log << text;
    //     return *this;
    // }
}