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
// Note: should implement hash table latter
#ifndef CTRAITS_H_INCLUDED_1509
#define CTRAITS_H_INCLUDED_1509
#pragma once

#include "Constants.h"

#include <string>

namespace WhoreMasterRenewal
{

class cTraits;
extern cTraits g_Traits;

// Represents a single trait
typedef struct sTrait
{
	char* m_Name;		// the name and unique ID of the trait
	char* m_Desc;		// a description of the trait

	sTrait* m_Next;		// the next trait in the list

	sTrait();
	~sTrait();
	
	sTrait( const sTrait& ) = delete;
	sTrait& operator = ( const sTrait& ) = delete;
	
}sTrait;

// Manages and loads the traits file
class cTraits
{
public:
	cTraits();
	~cTraits();
    
    cTraits( const cTraits& ) = delete;
	cTraits& operator = ( const cTraits& ) = delete;
    
	void Free();	// Delete all the loaded data

	void LoadTraits(std::string filename);	// Loads the traits from a file (adding them to the existing traits)
	void SaveTraits(std::string filename);	// Saves the traits to a file
	void AddTrait(sTrait* trait);
	void RemoveTrait(std::string name);
	sTrait* GetTrait(std::string name);
	sTrait* GetTraitNum(int num);
	int GetNumTraits() {return m_NumTraits;}

private:
	int m_NumTraits;
	sTrait* m_ParentTrait;				// the first trait in the list
	sTrait* m_LastTrait;				// the last trait in the list
};

} // namespace WhoreMasterRenewal

#endif // CTRAITS_H_INCLUDED_1509
