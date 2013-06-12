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
#ifndef CDUNGEON_H_INCLUDED_1532
#define CDUNGEON_H_INCLUDED_1532
#pragma once

#include <string>
#include <fstream>
#include <vector>

class Girl;
class cGirlTorture;
class TiXmlElement;
class TiXmlHandle;

// Keeps track of customers in the dungeon
typedef struct sDungeonCust
{
	sDungeonCust();				// constructor
	~sDungeonCust();			// destructor

	bool			m_Feeding;	// are you feeding them
	bool			m_Tort;		// if true then have already tortured today
	int				m_Reason;	// the reason they are here
	int				m_Weeks;	// the number of weeks they have been here

	// customer data
	int				m_NumDaughters;
	bool			m_HasWife;
	sDungeonCust*	m_Next;
	sDungeonCust*	m_Prev;
	int				m_Health;
	void OutputCustDetailString(std::string& Data, const std::string& detailName);
}sDungeonCust;

// Keeps track of girls in the dungeon
typedef struct sDungeonGirl
{
	sDungeonGirl();						// constructor
	~sDungeonGirl();					// destructor

	bool			m_Feeding;			// are you feeding them
	int				m_Reason;			// the reason they are here
	int				m_Weeks;			// the number of weeks they have been here

	// customer data
	Girl*			m_Girl;
	sDungeonGirl*	m_Next;
	sDungeonGirl*	m_Prev;
	void OutputGirlDetailString(std::string& Data, const std::string& detailName);
}sDungeonGirl;


// The dungeon
class cDungeon
{
private:
	sDungeonGirl* m_Girls;
	sDungeonGirl* m_LastDGirl;
	sDungeonCust* m_Custs;
	sDungeonCust* m_LastDCusts;
	unsigned long m_NumberDied;				// the total number of people that have died in the players dungeon
	int m_NumGirls;
	int m_NumCusts;

	int m_NumGirlsTort;						//	WD:	Tracking for Torturer
	int m_NumCustsTort;
	void updateGirlTurnDungeonStats(sDungeonGirl* d_girl);

public:
	cDungeon();								// constructor
	~cDungeon();							// destructor
	void Free();
	TiXmlElement* SaveDungeonDataXML(TiXmlElement* pRoot);	// saves dungeon data
	bool LoadDungeonDataXML(TiXmlHandle hDungeon);
	void LoadDungeonDataLegacy(std::ifstream& ifs);	// loads dungeon data
	void AddGirl(Girl* girl, int reason);
	void AddCust(int reason, int numDaughters, bool hasWife);
	void OutputGirlRow(int i, std::string* Data, const std::vector<std::string>& columnNames);
	void OutputCustRow(int i, std::string* Data, const std::vector<std::string>& columnNames);
	sDungeonGirl* GetGirl(int i);
	sDungeonGirl* GetGirlByName(std::string name);
	sDungeonCust* GetCust(int i);
	int GetDungeonPos(Girl* girl);
	Girl* RemoveGirl(Girl* girl);
	Girl* RemoveGirl(sDungeonGirl* girl);	// releases or kills a girl
	void RemoveCust(sDungeonCust* cust);	// releases or kills a customer
	void Update();

	int GetGirlPos(Girl* girl);
	int GetNumCusts()				{ return m_NumCusts; }
	int GetNumGirls()				{ return m_NumGirls; }
	unsigned long GetNumDied()		{ return m_NumberDied; }

	int NumGirlsTort()				{ return m_NumGirlsTort; }
	int NumGirlsTort(int n)			{ m_NumGirlsTort += n; return m_NumGirlsTort; }
 	int NumCustsTort()				{ return m_NumCustsTort; }
	int NumCustsTort(int n)			{ m_NumCustsTort += n; return m_NumCustsTort; }


	// WD:	Torturer tortures dungeon girl.
	//void doTorturer(sDungeonGirl* d_girl, Girl* t_girl, std::string& summary);	{ cGirlTorture::cGirlTorture(d_girl, t_girl) }

	void PlaceDungeonGirl(sDungeonGirl* newGirl);
	void PlaceDungeonCustomer(sDungeonCust* newCust);
};


#endif  // CDUNGEON_H_INCLUDED_1532