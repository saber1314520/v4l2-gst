#include "v4l2IOctrl.h"

namespace gpptest
{
    static const char *prefixes[] = {
        "video",
        "radio",
        "vbi",
        "swradio",
        "v4l-subdev",
        "v4l-touch",
        "media",
        NULL};

    bool V4l2DeviceList::is_v4l_dev(const char *name)
    {
        for (unsigned i = 0; prefixes[i]; i++)
        {
            unsigned l = strlen(prefixes[i]);

            if (!memcmp(name, prefixes[i], l))
            {
                if (isdigit(name[l]))
                    return true;
            }
        }
        return false;
    }

    int V4l2DeviceList::calc_node_val(const char *s)
    {
        int n = 0;

        s = std::strrchr(s, '/') + 1;

        for (unsigned i = 0; prefixes[i]; i++)
        {
            unsigned l = strlen(prefixes[i]);

            if (!memcmp(s, prefixes[i], l))
            {
                n = i << 8;       //i*2^8
                n += atol(s + l); //get 1 in video1
                return n;
            }
        }
        return 0;
    }

    bool V4l2DeviceList::sort_on_device_name(const std::string &s1, const std::string &s2)
    {
        int n1 = calc_node_val(s1.c_str());
        int n2 = calc_node_val(s2.c_str());

        return n1 < n2;
    }

    bool V4l2DeviceList::getDeviceList()
    {
        m_files.clear();
        m_links.clear();
        m_cards.clear();

        DIR *dp = opendir("/dev");
        struct dirent *ep;

        if (nullptr == dp)
        {
            printf("Cannot open the directory!");
            return false;
        }

        //travering all files in "/dev" and picking out all v4l2 devices
        while ((ep = readdir(dp)))
        {
            if (is_v4l_dev(ep->d_name))
            {
                m_files.push_back(std::string("/dev/") + ep->d_name);
            }
        }

        closedir(dp);

        for (auto iter = m_files.begin(); iter != m_files.end();)
        {
            char link[64 + 1];
            int link_len;
            std::string target;

            link_len = readlink(iter->c_str(), link, 64);

            //not a link or error
            if (link_len < 0)
            {
                iter++;
                continue;
            }
            link[link_len] = '\0';

            //if target is in files,remove it from files
            if (link[0] != '/')
            {
                target = "/dev/";
            }
            target += link;
            if (find(m_files.begin(), m_files.end(), target) == m_files.end())
            {
                iter++;
                continue;
            }

            //move the device node from files to links
            if (m_links[target].empty())
            {
                m_links[target] = *iter;
            }
            else
            {
                m_links[target] += ", " + *iter;
            }
            iter = m_files.erase(iter);
        }

        //just sort by sort_on_device_name
        std::sort(m_files.begin(), m_files.end(), sort_on_device_name);

        for (auto iter = m_files.begin(); iter != m_files.end(); ++iter)
        {
            int fd = open(iter->c_str(), O_RDWR); //fcntl.h for doc operation
            std::string bus_info;
            std::string card;

            if (fd < 0)
            {
                continue;
            }

            int err = ioctl(fd, VIDIOC_QUERYCAP, &vcap);
            if (err)
            {
                struct media_device_info mdi;

                err = ioctl(fd, MEDIA_IOC_DEVICE_INFO, &mdi);
                if (!err)
                {
                    if (mdi.bus_info[0])
                    {
                        bus_info = mdi.bus_info;
                    }
                    else
                    {
                        bus_info = std::string("platform:") + mdi.driver;
                    }

                    if (mdi.model[0])
                    {
                        card = mdi.model;
                    }
                    else
                    {
                        card = mdi.driver;
                    }
                }
            }
            else
            {
                bus_info = reinterpret_cast<const char *>(vcap.bus_info);
                card = reinterpret_cast<const char *>(vcap.card);
            }
            close(fd);

            if (err)
            {
                continue;
            }
            if (m_cards[bus_info].empty())
            {
                m_cards[bus_info] += card + "(" + bus_info + "):\n";
            }
            m_cards[bus_info] += "\t" + (*iter);
            if (!(m_links[*iter].empty()))
            {
                m_cards[bus_info] += " <- " + m_links[*iter];
            }
            m_cards[bus_info] += "\n";
        }

        m_devnum=m_cards.size();
        return true;
        
    }

    bool V4l2DeviceList::showDeviceList()
    {
        printf("There are %d devices ready!\n\n",m_devnum);


        for (auto iter = m_cards.begin(); iter!=m_cards.end(); ++iter)
        {
            printf("%s\n",iter->second.c_str());
            fflush(stdout);
        }
        return true;
    }

    bool V4l2DeviceList::isListEmpty()
    {
        return m_devnum<=0;
    }

    bool V4l2DeviceList::updateDeviceList()
    {
        this->getDeviceList();
        if (m_devnum_before!=m_devnum)
        {
            printf("\nWarning:devices connection status changed!\n\n");
            fflush(stdout);
            this->showDeviceList();
            m_devnum_before=m_devnum;
            return true;//changed
        }
        else
        {
            return false;//notchanged
        }
        
    }

    bool V4l2DeviceList::showFile()
    {
        if(!m_devnum)return false;
        printf("Available video source list:\n");
        for (int i = 0; i < m_devnum; ++i)
        {
            printf("%s\n",m_files[i].c_str());
        }
        return true;
    }

    std::string V4l2DeviceList::findFile()
    {
        if(!m_devnum)return std::string("");
        return m_files[0];
    }
}