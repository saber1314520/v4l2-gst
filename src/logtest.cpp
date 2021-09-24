#include "log.h"
#include <string>

using namespace gpptest;
using std::string;

int main()
{
    log log_info;

    string filename("todaytomorrow.txt");

    log_info.open(filename);

    log_info<<"Hello world!\n"<<"It is a nice day!\n";

    return true;
}