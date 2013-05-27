
#include "IconSurface.h"
#include "CLog.h"

IconSurface::IconSurface( std::string name )
    : CSurface( ImagePath(name + ".png") )
{
}
IconSurface::IconSurface( std::string name, const char*pt , const char* ext )
    : CSurface()
{
    std::string full = "";
    full += name;
    full += pt;
    full += ext;
    ImagePath dp(full);

    g_LogFile.ss() << "IconSurface::IconSurface\n"
         << "	name = " << name << "\n"
         << "	ext  = " << ext  << "\n"
         << "	full = " << full  << "\n"
         << "	dp   = " << dp.c_str()  << "\n"
    ;
    g_LogFile.ssend();

    LoadImage(dp.c_str(), true);
}



ButtonSurface::ButtonSurface( std::string name )
    : CSurface( ButtonPath(name + ".png") )
{
}
ButtonSurface::ButtonSurface (std::string name, const char* pt, const char* ext )
    : CSurface( ButtonPath((name + pt) + ext).c_str() )
{
}



ImageSurface::ImageSurface( std::string name )
    : CSurface( ImagePath(name + ".png") )
{
}
ImageSurface::ImageSurface ( std::string name, const char* pt, const char* ext )
    : CSurface( ImagePath((name + pt) + ext).c_str() )
{
}
