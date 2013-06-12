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

#ifndef MASTERFILE_H_INCLUDED_1501
#define MASTERFILE_H_INCLUDED_1501
#pragma once

#include <map>
#include <string>

class TiXmlHandle;
class TiXmlElement;

namespace WhoreMasterRenewal
{

class MasterFile
{
public:
	void LoadLegacy( std::string );
	bool exists( std::string );
	size_t size();
	void add( std::string );
	bool LoadXML( TiXmlHandle );
	TiXmlElement* SaveXML( TiXmlElement* );

private:
	typedef std::map<std::string,int> FileFlags;
	FileFlags files;
};

} // namespace WhoreMasterRenewal

#endif // MASTERFILE_H_INCLUDED_1501
