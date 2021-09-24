#include "v4l2IOctrl.h"

#include <thread>

using namespace v4l2IOctrl;
using std::ref;
using std::thread;


void working(const bool &flag)
{
    while (true)
    {
        if (flag)
        {
            printf("\n");
            for (int i = 0; i < 50; ++i)
            {
                printf("working:");
                for (int j = 0; j < 50; ++j)
                {
                    if (j == i)
                    {
                        printf("^");
                        if (49 == j)
                        {
                            printf("\r");
                            fflush(stdout);
                        }
                    }
                    else
                    {
                        printf("-");
                        if (49 == j)
                        {
                            printf("\r");
                            fflush(stdout);
                        }
                    }
                }
                if (49 == i)
                {
                    i = -1;
                }
                if (!flag)
                {
                    break;
                }
                usleep(100000);
            }    
        }
        else
        {
            printf("\n");
            while (true)
            {
                sleep(1);
                printf("sleeping\r");
                fflush(stdout);
                if (flag)
                {
                    break;
                }
            }
        }
    }
}

void checking(bool &flag, V4l2DeviceList &dlist)
{
    while (true)
    {
        dlist.updateDeviceList();
        flag =!dlist.isListEmpty();
        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    V4l2DeviceList testlist;

    bool workStatus = !testlist.isListEmpty(); //true for working,false for sleeping

    //thread2 checking
    thread threadchecking(checking, ref(workStatus), ref(testlist));
    threadchecking.detach();

    //thread1 working
    thread threadworking(working, ref(workStatus));
    threadworking.join();

    //working(workStatus);//test working
    // checking(workStatus,testlist);
    return 0;
}