#ifndef HELPER_H_INCLUDED_1917
#define HELPER_H_INCLUDED_1917
#pragma once

#include <string>
#include <sstream>
#include <cmath>

namespace WhoreMasterRenewal
{

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

//{ formatString implementation
namespace
{

std::stringstream& imp_formatString( std::stringstream& ss, const char* format );
std::stringstream& imp_formatString( std::stringstream& ss, const char* format )
{
    ss << format;
    
    return ss;
}

template<class T, class... VTtypes>
std::stringstream& imp_formatString( std::stringstream& ss, const char* format, const T& value, const VTtypes&... VTvalues );
template<class T, class... VTtypes>
std::stringstream& imp_formatString( std::stringstream& ss, const char* format, const T& value, const VTtypes&... VTvalues )
{
    for( ; *format != '\0'; format++ )
    {
        if( *format == '%' )
        {
            // check for %%
            if( *(format+1) == '%' )
            {
                ss << '%';
                format++;
                continue;
            }
            ss << value;
            return imp_formatString( ss, format+1, VTvalues... );
        }
        // You can use {%} to display parameter. It will not look for following '%'
        else if( *format == '{' && *(format+1) == '%' && *(format+2) == '}' )
        {
            ss << value;
            return imp_formatString( ss, format+3, VTvalues... );
        }
        
        ss << *format;
    }
    
    return ss;
}

}
//} formatString implementation

inline std::string formatString( const char* format )
{
    std::stringstream ss;
    return imp_formatString( ss, format ).str();
}

template<class T, class... VTtypes>
std::string formatString( const char* format, const T& value, const VTtypes&... VTvalues )
{
    std::stringstream ss;
    return imp_formatString( ss, format, value, VTvalues... ).str();
}

} // namespace WhoreMasterRenewal

#endif // HELPER_H_INCLUDED_1917
