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
#ifndef CSCREENBANK_H_INCLUDED_1519
#define CSCREENBANK_H_INCLUDED_1519
#pragma once

#include "cInterfaceWindow.h" // required inheritance

class cScreenBank;
extern cScreenBank g_BankScreen;

class cScreenBank : public cInterfaceWindowXML
{
public:

private:
	bool	GetDeposit;
	bool	GetWithdraw;
	char	buffer[256];

	static bool ids_set;
/*
 *	interface/event IDs
 */
	int back_id;		// Back button
	int deposit_id;		// Deposit button
	int depositall_id;	// Deposit All button
	int withdraw_id;	// Withdraw button
	int details_id;		// Bank Details text
	int header_id;		// page header text ("Town Bank")

	void set_ids();
public:
	cScreenBank();
	virtual ~cScreenBank();

	void init();
	void process();
	void check_events();
};

#endif // CSCREENBANK_H_INCLUDED_1519
