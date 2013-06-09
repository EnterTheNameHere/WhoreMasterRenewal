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

#include "sFacilityList.h"
#include "sFacility.h"
#include "DirPath.h"
#include "FileList.h"
#include "CLog.h"
#include "XmlMisc.h"

std::vector<sFacility *> *sFacilityList::list = nullptr;

sFacilityList::sFacilityList()
{
	if(list) {
		return;
	}
	list = new std::vector<sFacility *>;
/*
 *	build a list of XML room definition files
 */
 	FileList fl(DirPathR() << "Data", "*.roomsx");
/*
 *	loop through files and load them
 */
 	for(int i = 0; i < fl.size(); i++) {
/*
 *		get the file name
 */
	    std::string file = fl[i].full();
/*
 *		try and load it
 */
		if(load_xml(file) == true) {
			continue;
		}
/*
 *		moan if we failed at that task
 */
		g_LogFile.ss()<< "Error: failed to parse file '"
			<< file
			<< "': continuing..."
		;
		g_LogFile.ssend();
	}
}


bool sFacilityList::load_xml(std::string filename)
{
	TiXmlDocument doc(filename);
	if(!doc.LoadFile()) {
		g_LogFile.ss()<< "sFacilityList::load_xml: load failure for file '"
			<< filename
			<< "'"
			<< std::endl
		;
		g_LogFile.ss()<< "Error: line "
			<< doc.ErrorRow()
			<< ", col "
			<< doc.ErrorCol()
			<< ": "
			<< doc.ErrorDesc()
		;
		g_LogFile.ssend();
		return false;
	}
/*
 *	get the docuement root
 */
	TiXmlElement *el, *root_el = doc.RootElement();
/*
 *	loop over the elements attached to the root
 */
	for(	el = root_el->FirstChildElement();
		el ;
		el = el->NextSiblingElement()
	) {
/*
 *		create a new facility
 */
 		sFacility *fpt = new sFacility();
/*
 *		walk the XML tree to get the data
 */
		fpt->load_from_xml(el);
/*
 *		and add to the list
 */
		list->push_back(fpt);
	}
	return true;
}

