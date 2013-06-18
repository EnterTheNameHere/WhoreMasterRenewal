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
#ifndef CCUSTOMERS_H_INCLUDED_1532
#define CCUSTOMERS_H_INCLUDED_1532
#pragma once

#include "Constants.h"

namespace WhoreMasterRenewal
{

class cCustomers;
extern cCustomers g_Customers;

class Brothel;

// customers are randomly generated
typedef struct sCustomer
{
public:
    sCustomer( const sCustomer& ) = delete;
	sCustomer& operator = ( const sCustomer& ) = delete;
    
	// Regular Stats
	unsigned char m_IsWoman = 0;	// 0 means a man, 1 means a woman
	unsigned char m_Amount = 0;	// how many customers this represents
	unsigned char m_Class = 0;	// is the person rich, poor or middle class
	unsigned char m_Official = 0;	// is the person an official of the town

	unsigned int m_Money = 0;

	unsigned char m_Stats[NUM_STATS];
	unsigned char m_Skills[NUM_SKILLS];

//	unsigned char m_Skills[NUM_SKILLS];

	unsigned char m_Fetish = 0;	// the customers fetish
	unsigned char m_SexPref = 0;	// their sex preference

	unsigned char m_ParticularGirl = 0;	// the id of the girl he wants

	sCustomer* m_Next = nullptr;
	sCustomer* m_Prev = nullptr;
	
	sCustomer();
	~sCustomer();
	
	int happiness() {
		return m_Stats[STAT_HAPPINESS];
	}
}sCustomer;

class cCustomers
{
public:
	cCustomers();
	~cCustomers();

	void Free();

	void GenerateCustomers(Brothel*, int DayNight = 0);	// generates a random amount of possible customers based on the number of poor, rich, and middle class
//	sCustomer* GetParentCustomer();		// Gets a random customer from the customer base
	void GetCustomer(sCustomer& customer, Brothel* brothel);
	void ChangeCustomerBase();	// Changes customer base, it is based on how much money the player is bring into the town
	int GetNumCustomers() {return m_NumCustomers;}
	void AdjustNumCustomers(int amount) {m_NumCustomers+=amount;}
//	void Remove(sCustomer* cust);
//	int GetHappiness();	//mod

private:
	int m_Poor = 0;	// percentage of poor people in the town
	int m_Middle = 0;	// percentage of middle class people in the town
	int m_Rich = 0;	// percentage of rich people in the town

	int m_NumCustomers = 0;
//	sCustomer* m_Parent;
//	sCustomer* m_Last;
};

} // namespace WhoreMasterRenewal

#endif // CCUSTOMERS_H_INCLUDED_1532
