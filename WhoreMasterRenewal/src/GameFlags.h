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
#ifndef GAMEFLAGS_H_INCLUDED_1506
#define GAMEFLAGS_H_INCLUDED_1506
#pragma once

#include "Constants.h"

// Externs
extern unsigned int g_GameFlags[NUM_GAMEFLAGS][2];

inline void InitGameFlags()
{
	for(int i=0; i<NUM_GAMEFLAGS; i++)
	{
		g_GameFlags[i][0] = 0;
		g_GameFlags[i][1] = 0;
	}
}

inline void SetGameFlag(int flag, int value = 0)
{
	if(flag >= NUM_GAMEFLAGS)
		return;
	g_GameFlags[flag][0] = 1;
	g_GameFlags[flag][1] = value;
}

inline void ClearGameFlag(int flag)
{
	if(flag >= NUM_GAMEFLAGS)
		return;
	g_GameFlags[flag][0] = 0;
	g_GameFlags[flag][1] = 0;
}

inline bool CheckGameFlag(int flag)
{
	if(g_GameFlags[flag][0] == 1)
		return true;
	return false;
}

inline int GetGameFlag(int flag)
{
	return g_GameFlags[flag][1];
}
/*
inline void InitGameFlags();
inline void SetGameFlag(int flag, int value);
inline void ClearGameFlag(int flag);
inline int GetGameFlag(int flag);
inline bool CheckGameFlag(int flag);
*/
#endif
