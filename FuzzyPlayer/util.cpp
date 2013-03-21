#include "util.h"

std::vector<char> convertToCharStar(std::string str)
{
    std::vector<char> writable(str.size() + 1);
    copy(str.begin(), str.end(), writable.begin());
    writable.push_back('\0');
    return writable;
    //writable.push_back('\0'); char * c = &writable[0];
    // get the char* using &writable[0] or &*writable.begin()
}
