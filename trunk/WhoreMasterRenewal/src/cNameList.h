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
#ifndef CNAMELIST_H_INCLUDED_1522
#define CNAMELIST_H_INCLUDED_1522
#pragma once

#include <vector>
#include <string>

class cNameList
{
public:
    cNameList();
    cNameList( std::string file );
    
    std::string random();
    void load( std::string file );
    
private:
    std::vector<std::string> names;
};

class cDoubleNameList
{
public:
    cDoubleNameList();
    cDoubleNameList( std::string first, std::string last );
    void load( std::string first, std::string last );
    std::string random();
    
private:
    cNameList m_first;
    cNameList m_last;
};

#endif // CNAMELIST_H_INCLUDED_1522
