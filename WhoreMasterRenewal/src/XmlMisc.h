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
						   int statMods[] = nullptr,
						   int tempStats[] = nullptr);

TiXmlElement* SaveSkillsXML(TiXmlElement* pRoot,
							unsigned char skills[],
							int skillMods[] = nullptr,
							int tempSkills[] = nullptr);

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
							   unsigned char isEquipped[] = nullptr,
							   unsigned char quantities[] = nullptr );







bool LoadStatsXML(TiXmlHandle hStats,
				  unsigned char stats[],
				  int statMods[] = nullptr,
				  int tempStats[] = nullptr );

bool LoadSkillsXML(TiXmlHandle hSkills,
				   unsigned char skills[],
				   int skillMods[] = nullptr,
				   int tempSkills[] = nullptr );

bool LoadJobsXML(TiXmlHandle hJobs,
				 int buildingQualities[]);

bool LoadTraitsXML(TiXmlHandle hTraits,
				   unsigned char& numTraits,
				   sTrait* traits[],
				   unsigned char tempTraits[] = nullptr );

bool LoadActionsXML(TiXmlHandle hActions,
					int enjoyments[]);

bool LoadInventoryXML(TiXmlHandle hInventory,
					  sInventoryItem* items[],
					  int& numItems,
					  unsigned char isEquipped[] = nullptr,
					  unsigned char quantities[] = nullptr );

#endif // XMLMISC_H_INCLUDED_1534
