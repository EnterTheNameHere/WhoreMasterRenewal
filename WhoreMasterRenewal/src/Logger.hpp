#ifndef LOGGER_H_INCLUDED_0700
#define LOGGER_H_INCLUDED_0700
#pragma once

#include <string>
#include <fstream>

namespace WhoreMasterRenewal
{

class Logger
{
public:
    Logger( bool append = true, const std::string& filename = "GameLog.txt" );
    ~Logger();
    
    Logger& operator << ( char value );
    Logger& operator << ( unsigned char value );
    Logger& operator << ( bool value );
    Logger& operator << ( short int value );
    Logger& operator << ( unsigned short int value );
    Logger& operator << ( int value );
    Logger& operator << ( unsigned int value );
    Logger& operator << ( long int value );
    Logger& operator << ( unsigned long int value );
    Logger& operator << ( long long int value );
    Logger& operator << ( unsigned long long int value );
    Logger& operator << ( float value );
    Logger& operator << ( double value );
    Logger& operator << ( long double value );
    
    template<typename T>
    Logger& operator << ( T& value );
    
    Logger& operator << ( std::string value );
    
    Logger& operator << ( const char* value );
    Logger& operator << ( const unsigned char* value );
    
    Logger& operator << ( std::ios_base& (*func) (std::ios_base&) );
    Logger& operator << ( std::ios& (*func) (std::ios&) );
    Logger& operator << ( std::ostream& (*func) (std::ostream&) );
    
private:
    std::ofstream m_LogFileStream = {};
    //sf::Mutex m_Mutex;
    
    //static bool m_FirstRun;
    //static bool m_FailedToOpenLogFile;
};

} // namespace WhoreMasterRenewal

#endif // LOGGER_H_INCLUDED_0700
