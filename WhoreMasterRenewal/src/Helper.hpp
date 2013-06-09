#ifndef HELPER_H_INCLUDED_1917
#define HELPER_H_INCLUDED_1917
#pragma once

#include <string>
#include <sstream>
#include <cmath>

template<typename T> std::string toString( T value )
{
    std::stringstream str;
    str << value;
    std::string result;
    str >> result;
    return result;
}

template<typename T>
inline bool areEqual( T value1, T value2, T precision = 0.00001 )
{
    return ( std::abs( value1 - value2 ) <= precision );
}

#endif // HELPER_H_INCLUDED_1917
