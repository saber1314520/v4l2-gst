#include <iostream>

#include "v4l2IOctrl.h"

using namespace gpptest;

using std::cout;

int main()
{
     V4l2DeviceList testlist;

     testlist.showFile();
     
     return 0;
}