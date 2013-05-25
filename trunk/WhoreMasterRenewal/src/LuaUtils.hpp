
#ifndef LUAUTILS_HPP_INCLUDED_0702
#define LUAUTILS_HPP_INCLUDED_0702

#include <lua.hpp>

#include <memory>
#include <iomanip>
#include <sstream>

/*
lua_State* luaL_newstate( void );
void lua_close (lua_State *L);
void luaL_openlibs (lua_State *L);
void lua_setglobal (lua_State *L, const char *name);
void lua_pushboolean (lua_State *L, int b);
void lua_pushinteger (lua_State *L, lua_Integer n);
void lua_pushlightuserdata (lua_State *L, void *p);
//const char* lua_pushliteral (lua_State *L, const char *s);
//const char* lua_pushlstring (lua_State *L, const char *s, size_t len);
void lua_pushnil (lua_State *L);
void lua_pushnumber (lua_State *L, lua_Number n);
//const char* lua_pushstring (lua_State *L, const char *s);
int lua_pushthread (lua_State *L);
void lua_pushvalue (lua_State *L, int index);
//const char* lua_pushvfstring (lua_State *L, const char *fmt, va_list argp);
void lua_getglobal (lua_State *L, const char *name);
*/

namespace Lua
{

void luaD_DumpStack( lua_State* L );

void luaD_DumpStack( lua_State* L )
{
    if( L == 0 )
        L = nullptr;
    if( L == nullptr )
        return;
    
    int length = lua_gettop( L );
    
    std::cout << char(0xC9) << std::setw(7) << std::setfill( char(0xCD) ) << "" << " Lua Stack Dump ";
    std::cout << std::setw(8) << std::setfill( char(0xCD) ) << "" << "\n";
    std::cout << char(0xBA) << " Number of items in stack: " << length << "\n";
    
    for( int i = 1; i <= length; i++ )
    {
        std::cout << char(0xBA) << " [" << ((i<10)?"0":"") << i << "] ";
        
        int t = lua_type( L, i );
        switch( t )
        {
        case LUA_TSTRING:
            std::cout << "string:  '" << lua_tostring( L, i ) << "'\n";
            break;
            
        case LUA_TBOOLEAN:
            std::cout << "boolean: '" << lua_toboolean( L, i ) << "'\n";
            break;
            
        case LUA_TNUMBER:
            std::cout << "number:  '" << lua_tonumber( L, i ) << "'\n";
            break;
        
        case LUA_TFUNCTION:
            std::cout << "function\n";
            break;
            
        case LUA_TNIL:
            std::cout << "nil\n";
            break;
            
        case LUA_TNONE:
            std::cout << "none\n";
            break;
            
        case LUA_TLIGHTUSERDATA:
            std::cout << "ligth user data\n";
            break;
            
        case LUA_TTABLE:
            std::cout << "table\n";
            break;
            
        case LUA_TTHREAD:
            std::cout << "thread\n";
            break;
            
        case LUA_TUSERDATA:
            std::cout << "user data\n";
            break;
        
        default:
            std::cout << "unknown: '" << lua_typename( L, t ) << "'\n";
            break;
        }
    }
    
    std::cout << char(0xC8);
    std::cout << std::setw(31) << std::setfill( char(0xCD) ) << "";
    std::cout << std::endl;
}


class LuaRuntime
{
public:
    LuaRuntime( bool openLibs = true )
    {
        m_luaState = luaL_newstate();
        if( m_luaState == 0 )
            m_luaState = nullptr;
        
        if( openLibs )
            luaL_openlibs( m_luaState );
    }
    
    virtual ~LuaRuntime()
    {
        if( m_luaState != nullptr )
        {
            lua_close( m_luaState );
        }
    }
    
    LuaRuntime( LuaRuntime& ) = delete;
    LuaRuntime( const LuaRuntime& ) = delete;
    LuaRuntime& operator=( LuaRuntime& ) = delete;
    LuaRuntime& operator=( const LuaRuntime& ) = delete;
    
    template<typename T>
    void SetVariable( const std::string& variableName, const T& value, typename std::enable_if< std::is_integral<T>::value, T >::type* = 0 )
    {
        #ifdef DEBUG_LUA
            std::cout << "LuaRuntime: Registering variable under global name \"" << variableName << "\" as integer, value =\"" << value << "\"\n";
        #endif

        lua_pushinteger( m_luaState, value );
        lua_setglobal( m_luaState, variableName.c_str() );
    }
    
    template<typename T>
    void SetVariable( const std::string& variableName, const T& value, typename std::enable_if< std::is_floating_point<T>::value, T >::type* = 0 )
    {
        #ifdef DEBUG_LUA
            std::cout << "LuaRuntime: Registering variable under global name \"" << variableName << "\" as double, value =\"" << value << "\"\n";
        #endif
        
        lua_pushnumber( m_luaState, value );
        lua_setglobal( m_luaState, variableName.c_str() );
    }
    
    void SetVariable( const std::string& variableName, const std::string& value )
    {
        #ifdef DEBUG_LUA
            std::cout << "LuaRuntime: Registering variable under global name \"" << variableName << "\" as string, value =\"" << value << "\"\n";
        #endif

        lua_pushstring( m_luaState, value.c_str() );
        lua_setglobal( m_luaState, variableName.c_str() );
    }
    
    template<typename T>
        typename std::enable_if< std::is_integral<T>::value, T >::type
    GetVariable( const std::string& variableName )
    {
        #ifdef DEBUG_LUA
            std::cout << "LuaRuntime: Getting value of global variable named \"" << variableName << "\" as integer type (" << typeid(T).name() << "), ";
        #endif
        
        lua_getglobal( m_luaState, variableName.c_str() );
        T returnValue = lua_tointeger( m_luaState, -1 );
        lua_pop( m_luaState, 1 );
        
        #ifdef DEBUG_LUA
            std::cout << "value =\"" << returnValue << "\"\n";
        #endif
        
        return returnValue;
    }
    
    template<typename T>
        typename std::enable_if< std::is_floating_point<T>::value, T >::type
    GetVariable( const std::string& variableName )
    {
        #ifdef DEBUG_LUA
            std::cout << "LuaRuntime: Getting value of global variable named \"" << variableName << "\" as floating type (" << typeid(T).name() << "), ";
        #endif
        
        lua_getglobal( m_luaState, variableName.c_str() );
        T returnValue = lua_tonumber( m_luaState, -1 );
        lua_pop( m_luaState, 1 );
        
        #ifdef DEBUG_LUA
            std::cout << "value =\"" << returnValue << "\"\n";
        #endif
        
        return returnValue;
    }
    
    template<typename T>
        typename std::enable_if< std::is_same< T, std::string >::value, T >::type
    GetVariable( const std::string& variableName )
    {
        #ifdef DEBUG_LUA
            std::cout << "LuaRuntime: Getting value of global variable named \"" << variableName << "\" as string (" << typeid(T).name() << "), ";
        #endif
        
        lua_getglobal( m_luaState, variableName.c_str() );
        T returnValue = lua_tostring( m_luaState, -1 );
        lua_pop( m_luaState, 1 );
        
        #ifdef DEBUG_LUA
            std::cout << "value =\"" << returnValue << "\"\n";
        #endif
        
        return returnValue;
    }
    
    void RegisterFunction( const std::string& functionName, lua_CFunction fn )
    {
        #ifdef DEBUG_LUA
            std::cout << "LuaRuntime: Registering function under global name \"" << functionName << "\"\n";
        #endif
        
        lua_pushcclosure( m_luaState, fn, 0 );
        lua_setglobal( m_luaState, functionName.c_str() );
//        // (*f)(lua_State);
    }
    
    bool ExecuteString( std::string code )
    {
        if( !LoadString( code ) )
        {
            return false;
        };
        
        int status = lua_pcall( m_luaState, 0, 0, 0 );
        
        if( status != LUA_OK )
        {
            switch( status )
            {
            case LUA_ERRRUN:
                std::cout << "Runtime error:\n" << lua_tostring( m_luaState, -1 ) << "\nIn code:\n" << code << "\n";
                lua_pop( m_luaState, 1 );
                return false;
                break;
                                
            case LUA_ERRMEM:
                std::cout << "Memory allocation error:\n" << lua_tostring( m_luaState, -1 ) << "\nIn code:\n" << code << "\n";
                lua_pop( m_luaState, 1 );
                return false;
                break;
                
            case LUA_ERRGCMM:
                std::cout << "Garbage collector error:\n" << lua_tostring( m_luaState, -1 ) << "\nIn code:\n" << code << "\n";
                lua_pop( m_luaState, 1 );
                return false;
                break;
                
            default:
                std::cout << "Unknown error during running:\n\"" << code << "\".\n";
                return false;
                break;
            }
        }
        
        return true;
    }
    
    bool LoadString( std::string code )
    {
        std::istringstream stream( code );
        
        int status = lua_load( m_luaState, &LuaRuntime::ReaderFunction, &stream, "test", 0 );
        
        if( status != LUA_OK )
        {
            switch( status )
            {
            case LUA_ERRSYNTAX:
                std::cout << "Syntax error in \"" << code << "\".\n";
                return false;
                break;
                
            case LUA_ERRMEM:
                std::cout << "Memory allocation error during loading of code:\n" << lua_tostring( m_luaState, -1 ) << "\nCode:\n" << code << "\n";
                lua_pop( m_luaState, 1 );
                return false;
                break;
                
            case LUA_ERRGCMM:
                std::cout << "Garbage collector error during loading of code:\n" << lua_tostring( m_luaState, -1 ) << "\nCode:\n" << code << "\n";
                lua_pop( m_luaState, 1 );
                return false;
                break;
                
            default:
                std::cout << "Unknown error during loading \"" << code << "\".\n";
                return false;
                break;
            }
        }
        
        return true;
    }
    
    static const char* ReaderFunction( lua_State*, void* data, size_t *size )
    {
        static char buffer[8192];
        
        std::istringstream& stream = *(std::istringstream*)data;
        
        if( stream.eof() )
        {
            *size = 0;
            return nullptr;
        }
        
        stream.read( buffer, sizeof(buffer) );
        *size = stream.gcount();
        
        return buffer;
    }
    
    void Debug_DumpStack()
    {
        luaD_DumpStack( m_luaState );
    }

    
private:
    lua_State* m_luaState = {nullptr};
};


} // namespace WhoreMasterRenewal

#endif // LUAUTILS_HPP_INCLUDED_0702
