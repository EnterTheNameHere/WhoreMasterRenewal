
#include "Logger.hpp"

#include <iostream>
#include <string>

namespace WhoreMasterRenewal
{

Logger::Logger( const std::string& filename, bool append ) :
    m_LogFileStream()
{
    //if( m_FirstRun )
    //{
    //    sf::Lock lock( m_Mutex );
    //    {
    //        if( m_FirstRun )
    //        {
    //            m_FirstRun = false;
    //            
                if( append )
                    m_LogFileStream.open( filename, std::ios_base::out | std::ios_base::app );
                else
                    m_LogFileStream.open( filename, std::ios_base::out );
                
    //            if( !m_LogFileStream.is_open() )
    //            {
    //                m_FailedToOpenLogFile = true;
    //                
    //                std::cerr << "Error: Failed to open \"" << filename << "\" file for logging.\n"
    //                            << "Logging to file disabled. \n";
    //            }
    //        }
    //    }
    //}
}
Logger::~Logger()
{
    if( m_LogFileStream.is_open() )
    {
        m_LogFileStream.close();
    }
}

Logger& Logger::operator << ( char value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( unsigned char value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( bool value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( short int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( unsigned short int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}
        
Logger& Logger::operator << ( unsigned int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( long int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( unsigned long int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( long long int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( unsigned long long int value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( float value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( double value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( long double value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}


template<typename T>
Logger& Logger::operator << ( T& value )
{
    //sf::Lock lock( m_Mutex );
    {
        std::cout << value;
//               if( !m_FailedToOpenLogFile )
        if( m_LogFileStream.is_open() )
            m_LogFileStream << value;
    }
    
    return *this;
}

Logger& Logger::operator << ( std::string value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( const char* value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

Logger& Logger::operator << ( const unsigned char* value )
{
    std::cout << value;
    if( m_LogFileStream.is_open() )
        m_LogFileStream << value;
    
    return *this;
}

//bool Logger::m_FailedToOpenLogFile = false;
//bool Logger::m_FirstRun = true;

} // namespace WhoreMasterRenewal
