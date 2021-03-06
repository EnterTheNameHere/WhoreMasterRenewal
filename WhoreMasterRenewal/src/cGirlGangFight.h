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
#ifndef CGIRLGANGFIGHT_H_INCLUDED_1529
#define CGIRLGANGFIGHT_H_INCLUDED_1529
#pragma once

namespace WhoreMasterRenewal
{

class Girl;
struct sGang;

class cGirlGangFight
{
public:
	cGirlGangFight( Girl *girl );

	bool girl_fights()	{ return m_girl_fights; }
	bool girl_submits()	{ return !m_girl_fights; }
	bool girl_won()		{ return m_girl_wins; }
	bool girl_lost()	{ return !m_girl_wins; }
	bool player_won()	{ return m_player_wins; }

	bool wipeout()		{ return m_wipeout; }

//	int dead_goons()	{ return m_dead_goons; }

private:
    void lose_vs_own_gang(sGang* gang);
	void win_vs_own_gang(sGang* gang);
	int use_potions(sGang *gang, int casualties);
    
    
    
	Girl* m_girl;

	int m_girl_stats = 0;
	int m_goon_stats = 0;
	int m_max_goons = 0;
//	double m_ratio;
//	int m_dead_goons;

	bool m_girl_fights = false;
	bool m_girl_wins = false;
	bool m_player_wins = false;
	bool m_wipeout = false;
	bool m_unopposed = false;

	double m_odds = 0.0;
};

} // namespace WhoreMasterRenewal

#endif  // CGIRLGANGFIGHT_H_INCLUDED_1529
