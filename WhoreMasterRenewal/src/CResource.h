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
// CResource is a parent class for resource types used by CResourceManager

#ifndef CRESOURCE_H_INCLUDED_1521
#define CRESOURCE_H_INCLUDED_1521
#pragma once

namespace WhoreMasterRenewal
{

class CResource
{
public:
	virtual void Register(); // registers the resource with the resource manager
	virtual void Free(); // Free all data
	virtual void FreeResources(); // Frees only the loaded data, this is so the class isn't destroyed
	CResource();
	virtual ~CResource();
    
    CResource( const CResource& ) = delete;
	CResource& operator = ( const CResource& ) = delete;
    
	CResource* m_Next; // pointer to the next resource or null if end of list
	CResource* m_Prev; // Pointer to the previous resource or null if top of list
	unsigned long m_TimeUsed; // Stores the last time this resource was used
	bool m_Registered;
};

} // namespace WhoreMasterRenewal

#endif // CRESOURCE_H_INCLUDED_1521
