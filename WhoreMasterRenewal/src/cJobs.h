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
#ifndef CJOBS_H_INCLUDED_1524
#define CJOBS_H_INCLUDED_1524
#pragma once

#include <string>
#include <vector>

namespace WhoreMasterRenewal
{

class Girl;
class Brothel;
struct sGang;

typedef unsigned int u_int;

struct sJobs
{
    sJobs();
	~sJobs();
	
	void setup( std::string name,
                int min = 0,
                int max = -1,
                bool slave = true,
                bool free = true,
                char girlpay = 0,
                char up = 0,
                int id = 0,
                std::string like = "",
                std::string hate = "",
                int skill = 0 );
    
    std::string m_Name = "Default sJobs::m_Name";
	bool m_FreeGirlAble = false;
	bool m_SlaveAble = false;
	short m_MinPositions = 0;
	short m_MaxPositions = 0;
	char m_Upkeep = 0;
	char m_GirlPay = 0;
	int m_ActionID = 0;
    std::string m_LikeWork = "Default sJobs::m_LikeWork";
    std::string m_HateWork = "Default sJobs::m_HateWork";
	int m_DependantSkill = 0;
};

} // namespace WhoreMasterRenewal

#endif // CJOBS_H_INCLUDED_1524
