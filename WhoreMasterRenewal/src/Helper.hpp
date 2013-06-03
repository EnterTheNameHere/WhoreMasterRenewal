#ifndef HELPER_H_INCLUDED_1917
#define HELPER_H_INCLUDED_1917
#pragma once

#include <string>
#include <sstream>

template<typename T> std::string toString( T value )
{
    std::stringstream str;
    str << value;
    std::string result;
    str >> result;
    return result;
}

#endif // HELPER_H_INCLUDED_1917
