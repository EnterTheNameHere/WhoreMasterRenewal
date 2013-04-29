#ifndef UNITTEST_HPP_INCLUDED_1129
#define UNITTEST_HPP_INCLUDED_1129
#pragma once

#include <cmath>
#include <functional>
#include <map>

#include <boost/functional/factory.hpp>

#include <iostream>
#include <sstream>

#define DEBUG_UNITTEST_ASSERTION

namespace UnitTest
{

class NotEqualException : public std::exception
{
public:
    NotEqualException() : exception()
    {
        
    }
    
    virtual const char* what() const noexcept
    {
        return "Equal values expected, but values were not equal.";
    }
};

class EqualException : public std::exception
{
public:
    EqualException() : exception()
    {
        
    }
    
    virtual const char* what() const noexcept
    {
        return "Not equal values expected, but values were equal.";
    }
};

// Used when You need to explicitly say test should fail and assertions are not "faily" enough for You
class TestFailedException : public std::exception
{
public:
    TestFailedException() : exception()
    {
        
    }
    
    virtual const char* what() const noexcept
    {
        return "Test Failed";
    }
};


class Helper
{
public:
    Helper() = delete;
    Helper( const Helper& ) = delete;
    Helper( Helper&& ) = delete;
    Helper operator= ( const Helper& ) = delete;
    Helper operator= ( Helper&& ) = delete;
    virtual ~Helper();
    
    template<typename T>
    static bool AreEqual( T value1, T value2, T precission = 0.00001 )
    {
        T difference = std::abs( value1 - value2 );
        return ( difference <= precission );
    }
    
    template<typename T>
    static std::string GuessType( T )
    {
        /*
        signed integer type with width of
        exactly 8, 16, 32 and 64 bits respectively
        with no padding bits and using 2's complement for negative values
        (provided only if the implementation directly supports the type)
        int8_t
        int16_t
        int32_t
        int64_t
        
        fastest signed integer type with width of
        at least 8, 16, 32 and 64 bits respectively
        int_fast8_t
        int_fast16_t
        int_fast32_t
        int_fast64_t
        
        smallest signed integer type with width of
        at least 8, 16, 32 and 64 bits respectively
        int_least8_t
        int_least16_t
        int_least32_t
        int_least64_t
        
        maximum width integer type
        intmax_t
        
        integer type capable of holding a pointer
        intptr_t
        
        unsigned integer type with width of
        exactly 8, 16, 32 and 64 bits respectively
        (provided only if the implementation directly supports the type)
        uint8_t
        uint16_t
        uint32_t
        uint64_t
        
        fastest unsigned integer type with width of
        at least 8, 16, 32 and 64 bits respectively
        uint_fast8_t
        uint_fast16_t
        uint_fast32_t
        uint_fast64_t
        
        smallest unsigned integer type with width of
        at least 8, 16, 32 and 64 bits respectively
        uint_least8_t
        uint_least16_t
        uint_least32_t
        uint_least64_t
        
        maximum width unsigned integer type
        uintmax_t
        
        unsigned integer type capable of holding a pointer
        uintptr_t
        */
        
        if( std::is_same<bool, T>::value )
            return "(bool)";
        else if( std::is_same<signed char, T>::value )
            return "(signed char)";
        else if( std::is_same<unsigned char, T>::value )
            return "(unsigned char)";
        else if( std::is_same<char, T>::value )
            return "(char)";
        else if( std::is_same<wchar_t, T>::value )
            return "(wchar_t)";
        else if( std::is_same<char16_t, T>::value )
            return "(char16_t)";
        else if( std::is_same<char32_t, T>::value )
            return "(char32_t)";
        else if( std::is_same<short int, T>::value )
            return "(short int)";
        else if( std::is_same<unsigned short int, T>::value )
            return "(unsigned short int)";
        else if( std::is_same<int, T>::value )
            return "(int)";
        else if( std::is_same<unsigned int, T>::value )
            return "(unsigned int)";
        else if( std::is_same<long int, T>::value )
            return "(long int)";
        else if( std::is_same<unsigned long int, T>::value )
            return "(unsigned long int)";
        else if( std::is_same<long long int, T>::value )
            return "(long long int)";
        else if( std::is_same<unsigned long long int, T>::value )
            return "(unsigned long long int)";
        else if( std::is_same<float, T>::value )
            return "(float)";
        else if( std::is_same<double, T>::value )
            return "(double)";
        else if( std::is_same<long double, T>::value )
            return "(long double)";
        else
            return "(unknown)";
    }
};



class Assert
{
public:
    Assert() = delete;
    Assert( const Assert& ) = delete;
    Assert( const Assert&& ) = delete;
    Assert& operator= ( const Assert& ) = delete;
    ~Assert();
    
    
    
    template<typename T, class = typename std::enable_if< std::is_floating_point<T>::value >::type>
    static void Equals( T value1, T value2, T precision = 0.00001 )
    {
        bool areEqual = Helper::AreEqual( value1, value2, precision );
        
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << std::fixed << "Equals<floating_point " << Helper::GuessType(value1) << "\nT = \"" << value1 << "\", \nT = \"" << value2 << "\", \nT precision = \"" << precision << "\">\n";
        std::cout << std::fixed << "\t|difference| = \"" << std::abs( value1 - value2 ) << "\" RESULT = \"" << std::boolalpha << areEqual << "\"\n";
        #endif
        
        if( !areEqual )
        {
            throw NotEqualException();
        }
    }
    
    template<typename T, class = typename std::enable_if< std::is_floating_point<T>::value >::type>
    static void NotEquals( T value1, T value2, T precision = 0.00001 )
    {
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << std::fixed << "NotEquals<floating_point " << Helper::GuessType(value1) << "\nT = \"" << value1 << "\", \nT = \"" << value2 << "\", \nT precision = \"" << precision << "\">\n";
        #endif
        
        bool areNotEqual = false;
        
        // NotEqualsException expected, since we're calling Equals
        try
        {
            Equals( value1, value2, precision );
        }
        catch( NotEqualException& ex )
        {
            areNotEqual = true;
        }
        
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << std::fixed << "\t|difference| = \"" << std::abs( value1 - value2 ) << "\" RESULT = \"" << std::boolalpha << areNotEqual << "\"\n";
        #endif
        
        if( !areNotEqual )
        {
            throw EqualException();
        }
    }
    
    template<typename T, class = typename std::enable_if< std::is_integral<T>::value >::type>
    static void Equals( T value1, T value2 )
    {
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << "Equals<integral " << Helper::GuessType(value1) << "\n\tT = \"" << value1 << "\", \n\tT = \"" << value2 << "\">\n";
        #endif
        
        bool areEqual = (value1 == value2);
        
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << "\t|difference| = \"" << std::abs( value1 - value2 ) << "\", RESULT = \"" << std::boolalpha << areEqual << "\"\n" ;
        #endif
        
        if( !areEqual )
        {
            throw NotEqualException();
        }
    }
    
    template<typename T, class = typename std::enable_if< std::is_integral<T>::value >::type>
    static void NotEquals( T value1, T value2 )
    {
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << "NotEquals<integral " << Helper::GuessType(value1) << "\n\tT = \"" << value1 << "\", \n\tT = \"" << value2 << "\">\n";
        #endif
        
        bool areNotEqual = false;
        
        try
        {
            Equals( value1, value2 );
        }
        catch( NotEqualException& ex )
        {
            areNotEqual = true;
        }
        
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << "\t|difference| = \"" << std::abs( value1 - value2 ) << "\", RESULT = \"" << std::boolalpha << areNotEqual << "\"\n";
        #endif
        
        if( !areNotEqual )
        {
            throw EqualException();
        }
    }

    static void Equals( std::string value1, std::string value2 )
    {
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << std::fixed << "Equals<std::string = \"" << value1 << "\", std::string = \"" << value2 << "\">\n";
        #endif
        
        bool areEqual = ( value1 == value2 );
        
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << "\tRESULT = \"" << std::boolalpha << areEqual << "\"\n";
        #endif
        
        if( !areEqual )
        {
            throw NotEqualException();
        }
    }
    
    static void NotEquals( std::string value1, std::string value2 )
    {
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << std::fixed << "NotEquals<std::string = \"" << value1 << "\", std::string = \"" << value2 << "\">\n";
        #endif
        
        bool areNotEqual = true;
        try
        {
            Equals( value1, value2 );
        }
        catch( NotEqualException ex )
        {
            areNotEqual = false;
        }
        
        #ifdef DEBUG_UNITTEST_ASSERTION
        std::cout << "\tRESULT = \"" << std::boolalpha << areNotEqual << "\"\n";
        #endif
        
        if( !areNotEqual )
        {
            throw EqualException();
        }
    }
};



class TestResult
{
public:
    TestResult() = delete;
    TestResult( const TestResult& ) = delete;
    TestResult( TestResult&& ) = delete;
    TestResult operator= ( const TestResult& ) = delete;
    TestResult operator= ( TestResult&& ) = delete;
    virtual ~TestResult();
};

class ITest
{
public:
    virtual ~ITest() {};
    virtual void Run()
    {
        RunTest();
    };
    virtual void RunTest() const = 0;
    virtual const std::string& GetTestName() const = 0;
};

typedef std::function< ITest*() > TestFactory;

class TestManager
{
public:
    static int RegisterTest( TestFactory factoryFunc )
    {
        NumOfTests()++;
        
        #ifdef DEBUG_UNITTEST
        std::cout << "Registering test under id '" << NumOfTests() << "'\n";
        #endif
        
        TestFactories()[NumOfTests()] = factoryFunc;
        
        #ifdef DEBUG_UNITTEST
        std::cout << "Successfuly registered test under id '" << NumOfTests() << "'\n";
        #endif
        
        return NumOfTests();
    }
    
    void RunAllTests()
    {
        #ifdef DEBUG_UNITTEST
        std::cout << "Running tests\n";
        #endif
        
        for( auto factoryFunction : TestFactories() )
        {
            ITest* test = static_cast<ITest*>( factoryFunction.second() );
            
            try
            {
                std::cout << "Running test \"" << test->GetTestName() << "\"\n";
                test->Run();
                std::cout << "... Test \"" << test->GetTestName() << "\" successful\n";
            }
            catch( NotEqualException& ex )
            {
                std::cout << "... Test \"" << test->GetTestName() << "\" failed:\n" << ex.what() << "\n";
            }
            catch( EqualException& ex )
            {
                std::cout << "... Test \"" << test->GetTestName() << "\" failed:\n" << ex.what() << "\n";
            }
            catch( TestFailedException ex )
            {
                std::cout << "... Test \"" << test->GetTestName() << "\" failed:\n" << ex.what() << "\n";
            }
            catch(...)
            {
                std::cout << "... Test \"" << test->GetTestName() << "\" failed:\n{unknown}\n";
                throw;
            }
            
            delete test;
        }
    }
    
private:
    static std::map<int, TestFactory>& TestFactories()
    {
        // static pointer with no destruction intended
        static std::map<int, TestFactory>* ans_TestsFactories = new std::map<int, TestFactory>();
        return *ans_TestsFactories;
    }
    
    static int& NumOfTests()
    {
        static int ans_NumOfTests = 0;
        return ans_NumOfTests;
    }
};

template<typename Derived>
class Test : public ITest
{
public:
    Test()
    {
        #ifdef DEBUG_UNITTEST
        std::cout << "UnitTestFixture::UnitTestFixture() m_ID='" << static_cast<Derived*>(this)->m_ID << "'\n";
        #endif
    }
    
    virtual ~Test()
    {
        #ifdef DEBUG_UNITTEST
        std::cout << "UnitTestFixture::~UnitTestFixture() m_ID='" << static_cast<Derived*>(this)->m_ID << "'\n";
        #endif
    }
    /*
    virtual void RunTest() const override
    {
        #ifdef DEBUG_UNITTEST
        std::cout << "UnitTestFixture::RunTest() m_ID='" << static_cast<Derived*>(this)->m_ID << "'\n";
        #endif
    }
    */
    
    int getID() const
    {
        return m_ID;
    }
    
    const std::string& GetTestName() const
    {
        static std::string name = "";
        if( name == "" )
        {
            std::stringstream ss;
            
            // Determine the Derived class name, if available
            #if __GNUC__ // for GCC
                
                // GCC returns function name like
                //  "const string& Test<Derived>::GetTestName() [with Derived = ClassName; ]"
                std::string className = __PRETTY_FUNCTION__;
                size_t start = className.find( "Derived = " );
                size_t end = className.find( ";", start );
                
                if( start != std::string::npos && end != std::string::npos )
                {
                    start += 10; // we want to copy only the class name without "Derived = "
                    ss << className.substr( start, end - start );
                }
                else
                {
                    ss << "ID " << m_ID;
                }
                
            #else // unknown compiler
                
                ss << "ID " << m_ID; // use id of test when unable to get class name
                
            #endif
            
            name = ss.str();
        }
        return name;
    }
    
private:
    static const int m_ID;
};
template<typename Derived>
const int Test<Derived>::m_ID = TestManager::RegisterTest( boost::factory<Derived*>() );



} // namespace UnitTest

#endif // UNITTEST_HPP_INCLUDED_1129
