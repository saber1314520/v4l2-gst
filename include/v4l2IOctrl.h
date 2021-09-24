#pragma once

#include <dirent.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/media.h>
// #include <unistd.h>

#include <algorithm>
#include <vector>
#include <string>
#include <map>
#include <cstring>

namespace gpptest
{
    class V4l2DeviceList
    {
    private:
        int m_devnum;
        int m_devnum_before;
        std::vector<std::string> m_files;
        std::map<std::string,std::string> m_links;
        std::map<std::string,std::string> m_cards;
        struct v4l2_capability vcap;

    private:
        //for initializing
        bool getDeviceList();
        //check if a filename is a v4l2 device
        static bool is_v4l_dev(const char *name);

        static int calc_node_val(const char *s);
        //sort entrance
        static bool sort_on_device_name(const std::string &s1, const std::string &s2);

    public:
        V4l2DeviceList(){getDeviceList();showDeviceList();m_devnum_before=m_devnum;}
        ~V4l2DeviceList(){}
        bool showFile();//only for test
        std::string findFile();//return the first available video source file in m_file with full path
        bool showDeviceList();
        bool updateDeviceList();
        bool isListEmpty();
    };
}
