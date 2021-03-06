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

#include "cRng.h"
#include "sConfig.h"

#include <cstdlib>
#include <ctime>

namespace WhoreMasterRenewal
{

cRng::cRng()
{
    srand( static_cast<int>( time(nullptr) ) );
}

cRng::~cRng()
{
    
}

/*
 * easier to use the method internally than an operator
 * (OK - I could use (*this) % foo, but that's messy...)
 */
int cRng::random( int n )
{
    float scaling_factor = rand() / float(RAND_MAX);
    return static_cast<int>( scaling_factor * n );
}

int cRng::operator %( int n )
{
    return random(n);
}

int cRng::operator +( int n )
{
    return random(100) + n;
}

bool cRng::percent( int n )
{
    return ( 1 + random(100) ) < n;
}

int cRng::d100()
{
    return random(100) + 1;
}

int cRng::in_range( int min, int max, int range )
{
	int diff = max - min;

	if( diff == 0 )
        return max;
	if( diff < 0 )
        return random( range );
	
	return min + random( diff );
}

bool cRng::is_boy( int mod )
{
	cConfig cfg;

	int chance = 100 - cfg.pregnancy.chance_of_girl();
	chance += mod;
	
	if( chance > 100 )
        chance = 100;
	if( chance < 0 )
        chance = 0;
    
	return percent( chance );
}

bool cRng::is_girl( int mod )
{
	cConfig cfg;

	int chance = cfg.pregnancy.chance_of_girl();
	chance += mod;
	
	if( chance > 100 )
        chance = 100;
	if( chance < 0 )
        chance = 0;
    
	return percent( chance );
}

} // namespace WhoreMasterRenewal
