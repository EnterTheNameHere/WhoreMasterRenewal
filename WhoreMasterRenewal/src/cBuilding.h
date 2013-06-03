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
#ifndef CBUILDING_H_INCLUDED_1533
#define CBUILDING_H_INCLUDED_1533
#pragma once

#include <fstream>
#include <string>
#include <vector>

struct sFacility;

class cBuilding
{
public:
	cBuilding();
    
    int capacity();
	int free_space();
	int used_space();
    
	bool add( sFacility* );
	sFacility* remove(int);
	sFacility* item_at(int);
	sFacility* operator[](int);
	int	size();
	void commit();
	void revert();

	std::ofstream& save(std::ofstream& ofs, std::string building_name);
	std::ifstream& load(std::ifstream& ifs);
/*
 *	is the list free of changes that may need to be reverted?
 */
	bool list_is_clean();
/*
 *	copies the current list so we can revert all changes maed
 */
	void make_reversion_list();
	void clear_reversion_list();
	
private:
	int	m_capacity;
	int	m_free;
	typedef	std::vector<sFacility*> vFacilities;
	vFacilities	m_facilities;
	vFacilities* m_reversion;
};


#endif // CBUILDING_H_INCLUDED_1533
