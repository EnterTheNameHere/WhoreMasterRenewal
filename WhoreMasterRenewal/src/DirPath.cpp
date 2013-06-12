/*
 * Copyright 2009, 2010, The Pink Petal Development Team.
 * The Pink Petal Devloment Team are defined as the game's coders 
 * who meet on http://pinkpetal.co.cc
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DirPath.h"
#include "sConfig.h"

namespace WhoreMasterRenewal
{

/*
 * kind of trivial...
 */
#ifdef LINUX
const char	DirPath::sep[] = "/";
#else
const char	DirPath::sep[] = "\\";
#endif



DirPath::DirPath( const char* top_dir )
{
    path = top_dir;
    size_t pos = path.find( '\r' );
    
    if( pos != std::string::npos )
    {
        path[pos] = 0;
    }
}

DirPath::DirPath( DirPath& dp )
{
    path = dp.path;
}

DirPath::~DirPath() {}

DirPath& DirPath::operator << ( std::string& s )
{
    path += sep;
    path += s;
    return *this;
}

DirPath& DirPath::operator << ( const char* pt )
{
    path += sep;
    path += pt;
    return *this;
}

std::ostream& DirPath::operator << ( std::ostream& os )
{
    return os << path;
}

const char* DirPath::c_str()
{
    return path.c_str();
}

DirPath::operator const char* ()
{
    return path.c_str();
}

DirPath::operator std::string()
{
    return path;
}



DirPathR::DirPathR()
{
	cConfig cfg;
    std::string s = cfg.override_dir();

	(*this) << "." << "Resources";
	over = DirPath() << s;
}

DirPathR::DirPathR( DirPathR& dp )
    : DirPath( dp )
{
    path = dp.path;
    over = dp.over;
}

std::ifstream& DirPathR::open( std::ifstream& ifs )
{
    ifs.open( over.c_str() );
    
    if( !ifs.good() )
    {
        ifs.open( c_str() );
    }
    
    return ifs;
}

std::ofstream& DirPathR::open( std::ofstream& ofs )
{
    ofs.open( over.c_str() );
    
    if( !ofs.good() )
    {
        ofs.open( c_str() );
    }
    
    return ofs;
}
/*
 *  to build the path we use the << operator
 *  since that lets us cope with char * and string
 *  types mixed into sequences of arbitrary length
 *
 *  this is the string operator
 */
DirPath& DirPathR::operator << ( std::string& s )
{
    over << s;
    path += sep;
    path += s;
    return *this;
}
/*
 *  and here's one for const char * values
 */
DirPath& DirPathR::operator << ( const char* pt )
{
    over << pt;
    path += sep;
    path += pt;
    return *this;
}



ImagePath::ImagePath( std::string filename )
    : DirPath()
{
    ( *this ) << "Resources" << "Images" << filename;
}

ButtonPath::ButtonPath( std::string filename )
    : DirPath()
{
    ( *this ) << "Resources" << "Buttons" << filename;
}

ScriptPath::ScriptPath( std::string filename )
    : DirPath()
{
    ( *this ) << "Resources" << "Scripts" << filename;
}

} // namespace WhoreMasterRenewal
