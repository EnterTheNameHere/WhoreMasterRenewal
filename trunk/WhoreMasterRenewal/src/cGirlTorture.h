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
#ifndef CGIRLTORTURE_H_INCLUDED_1528
#define CGIRLTORTURE_H_INCLUDED_1528
#pragma once

#include <string>

struct	sGirl;
struct	sDungeonGirl;
class	cPlayer;
class	cDungeon;
class	CLog;
class	cConfig;

class cGirlTorture {
	cPlayer			*m_Player;
	sGirl			*m_Girl;
	sGirl			*m_Torturer;
	sDungeonGirl	*m_DungeonGirl;
	cDungeon		*m_Dungeon;
    std::string 		m_Message;
	bool			m_Fight;
	bool			m_TorturedByPlayer;

	bool			girl_escapes();
	void			AddTextPlayer();
	void			AddTextTorturerGirl();
	void			UpdateStats();
	void			UpdateTraits();
	void			add_trait(std::string trait, int pc);
	bool			IsGirlInjured(unsigned int unModifier);		// Based on cGirls::GirlInjured() 
	void			MakeEvent(std::string sMsg);
	void			DoTorture();

public:
	~cGirlTorture();							// destructor
	cGirlTorture(sGirl *pGirl);					// Torture Girl by player
	cGirlTorture(sDungeonGirl *pGirl);			// Torture Dungeon girl by player
	cGirlTorture(sDungeonGirl *pGirl, sGirl *pTourturer);	// Tortured by Torture job girl

};

#endif // CGIRLTORTURE_H_INCLUDED_1528
