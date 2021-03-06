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

#include "cScreenBank.h"
#include "BrothelManager.hpp"
#include "Brothel.hpp"
#include "cWindowManager.h"
#include "cMessageBox.h"
#include "cGold.h"
#include "cGetStringScreenManager.h"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"
#include "cInterfaceEvent.h"
#include "DirPath.h"

namespace WhoreMasterRenewal
{

bool cScreenBank::ids_set = false;

cScreenBank::cScreenBank() : cInterfaceWindowXML()
{
    DirPath dp = DirPath()
        << "Resources"
        << "Interface"
        << "bank_screen.xml"
    ;
    m_filename = dp.c_str();
}

cScreenBank::~cScreenBank()
{
    
}

void cScreenBank::set_ids()
{
	ids_set = true;
	back_id	= get_id("BackButton");
	details_id = get_id("BankDetails");
	header_id = get_id("ScreenHeader");
	deposit_id = get_id("DepositButton");
	depositall_id = get_id("DepositAllButton");
	withdraw_id = get_id("WithdrawButton");
}

void cScreenBank::init()
{
	if(!g_InitWin) {
		return;
	}
	Focused();
	g_InitWin = false;

	if(GetDeposit)
	{
		if(g_Gold.misc_debit(g_IntReturn))
			g_Brothels.DepositInBank(g_IntReturn);
		else
			g_MessageQue.AddToQue("You don't have that much!", 0);
		GetDeposit = false;
	}

	if(GetWithdraw)
	{
		if(g_IntReturn <= g_Brothels.GetBankMoney())
		{
			g_Brothels.WithdrawFromBank(g_IntReturn);
			g_Gold.misc_credit(g_IntReturn);
		}
		else
			g_MessageQue.AddToQue("There isn't that much in your account", 0);

		GetWithdraw = false;
	}

	std::locale syslocale("");
	std::stringstream ss;
	ss.imbue(syslocale);

	ss << "Bank account: " << g_Brothels.GetBankMoney() << " gold" << std::endl;
	ss << "On hand: " << g_Gold.ival() << " gold";

	EditTextItem(ss.str(), details_id);

	// disable/enable Withdraw button depending on whether player has money in bank
	DisableButton(withdraw_id, (g_Brothels.GetBankMoney() == 0));
	// likewise, if player has no money on hand, disable deposit buttons
	DisableButton(depositall_id, (g_Gold.ival() <= 0));
	DisableButton(deposit_id, (g_Gold.ival() <= 0));
}

void cScreenBank::process()
{
/*
 *	we need to make sure the ID variables are set
 */
	if(!ids_set)
		set_ids();

/*
 *	set up the window if needed
 */
	init();

/*
 *	check to see if there's a button event needing handling
 */
	check_events();
}


void cScreenBank::check_events()
{
/* 
 *	no events means we can go home
 */
	if(g_InterfaceEvents.GetNumEvents() == 0)
		return;

/*
 *	if it's the back button, pop the window off the stack
 *	and we're done
 */
	if(g_InterfaceEvents.CheckButton(back_id)) {
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}
	if(g_InterfaceEvents.CheckButton(depositall_id))
	{
		if(g_Gold.ival() <= 0)
		{
			g_MessageQue.AddToQue("You have no money to deposit.", 0);
			return;
		}
		GetDeposit = true;
		g_IntReturn = g_Gold.ival();
		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(deposit_id))
	{
		if(g_Gold.ival() <= 0)
		{
			g_MessageQue.AddToQue("You have no money to deposit.", 0);
			return;
		}
		GetDeposit = true;
		g_WinManager.Push(GetInt,&g_GetInt);
		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(withdraw_id))
	{
		GetWithdraw = true;
		g_WinManager.Push(GetInt,&g_GetInt);
		g_InitWin = true;
		return;
	}

}

} // namespace WhoreMasterRenewal
