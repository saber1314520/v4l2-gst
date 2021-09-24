#include <iostream>
#include <linux/videodev2.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc,char *argv[])
{
    int ret=0;
    int fd=open("/dev/video0",O_RDWR);
    struct v4l2_capability cap;
    
    ret=ioctl(fd,VIDIOC_QUERYCAP,&cap);

    printf("Driver Name:%s\nCard Name:%s\nBus info:%s\nDriver Version:%u.%u.%u\n",
    cap.driver,cap.card,cap.bus_info,(cap.version>>16)&0XFF, (cap.version>>8)&0XFF,cap.version&0XFF);

    // system("v4l2-ctl --list-devices");

    return 0;
}