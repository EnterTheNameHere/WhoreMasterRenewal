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
#ifndef XMLMISC_H_INCLUDED_1534
#define XMLMISC_H_INCLUDED_1534
#pragma once

#include "XmlUtil.h"

#define TIXML_USE_STL
#include "tinyxml.h"

#include <string>

struct sTrait;
struct sInventoryItem;

std::string XMLifyString(const std::string& XMLName);
std::string XMLifyString(const char* XMLName);
std::string UnXMLifyString(const std::string& XMLName);
std::string UnXMLifyString(const char* XMLName);

TiXmlElement* SaveStatsXML(TiXmlElement* pRoot,
						   unsigned char stats[],
						   int statMods[] = 0,
						   int tempStats[] = 0);

TiXmlElement* SaveSkillsXML(TiXmlElement* pRoot,
							unsigned char skills[],
							int skillMods[] = 0,
							int tempSkills[] = 0);

TiXmlElement* SaveJobsXML(TiXmlElement* pRoot,
						  int buildingQualities[]);

TiXmlElement* SaveTraitsXML(TiXmlElement* pRoot,
							std::string TagName,
							const int numTraits,
							sTrait* traits[],
							unsigned char tempTraits[]);

TiXmlElement* SaveActionsXML(TiXmlElement* pRoot,
							 int enjoyments[]);

TiXmlElement* SaveInventoryXML(TiXmlElement* pRoot,
							   sInventoryItem* items[],
							   const int numItems,
							   unsigned char isEquipped[] = 0,
							   unsigned char quantities[] = 0);







bool LoadStatsXML(TiXmlHandle hStats,
				  unsigned char stats[],
				  int statMods[] = 0,
				  int tempStats[] = 0);

bool LoadSkillsXML(TiXmlHandle hSkills,
				   unsigned char skills[],
				   int skillMods[] = 0,
				   int tempSkills[] = 0);

bool LoadJobsXML(TiXmlHandle hJobs,
				 int buildingQualities[]);

bool LoadTraitsXML(TiXmlHandle hTraits,
				   unsigned char& numTraits,
				   sTrait* traits[],
				   unsigned char tempTraits[] = 0);

bool LoadActionsXML(TiXmlHandle hActions,
					int enjoyments[]);

bool LoadInventoryXML(TiXmlHandle hInventory,
					  sInventoryItem* items[],
					  int& numItems,
					  unsigned char isEquipped[] = 0,
					  unsigned char quantities[] = 0);

#endif // XMLMISC_H_INCLUDED_1534
