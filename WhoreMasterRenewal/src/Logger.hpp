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
    Logger( const std::string& filename = "GameLog.txt", bool append = true );
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
    
private:
    std::ofstream m_LogFileStream;
    //sf::Mutex m_Mutex;
    
    //static bool m_FirstRun;
    //static bool m_FailedToOpenLogFile;
};

} // namespace WhoreMasterRenewal

#endif // LOGGER_H_INCLUDED_0700
