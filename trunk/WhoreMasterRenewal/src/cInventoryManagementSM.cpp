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

#include "cInventoryManagementSM.h"
#include "InterfaceGlobals.h"
#include "InterfaceIDs.h"
#include "InterfaceGlobals.h"
#include "cBrothel.h"
#include "DirPath.h"

#include <string>
#include <sstream>

extern cInterfaceEventManager g_InterfaceEvents;
extern sInterfaceIDs g_interfaceid;
extern	bool	g_InitWin;
extern	int	g_TalkCount;
extern	bool	g_Cheats;
extern	bool	g_UpArrow;
extern	bool	g_DownArrow;
extern	bool	eventrunning;

cInventoryManagementSM::cInventoryManagementSM()
{
    leftOwner	= 0;
    rightOwner	= 1;
    NumBrothelGirls	= 0;
    NumDungeonGirls	= 0;
    filter		= 0;
    leftItem	= 0;
    rightItem	= 0;
}

void cInventoryManagementSM::upd_item_lists()
{
	const int owners_l = g_interfaceid.LIST_ITMOWNERSL;
	const int owners_r = g_interfaceid.LIST_ITMOWNERSR;

	g_ItemManagement.SetSelectedItemInList(owners_l, leftOwner);
	g_ItemManagement.SetSelectedItemInList(owners_r, rightOwner);
}

void cInventoryManagementSM::handle_events()
{
/*
 *	The BACK button - leave the screen
 */
	if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_ITMBACK))
	{
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}

	if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_ITMSHIFTR))
	{
		item_shift_r();
		upd_item_lists();
		return;
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_ITMSHIFTL))
	{
		if((rightOwner != -1 && leftOwner != -1))
		{
			leftItem = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSL);
			rightItem = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR);
			if(rightOwner == 1)	// target is shop
			{
				sGirl* targetGirl = 0;
				if(leftOwner > 1)	// giving to a girl from the shop
				{
					if(leftOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
						targetGirl = g_Brothels.GetGirl(g_CurrBrothel, g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-2);
					else	// dungeon girl
						targetGirl = (g_Brothels.GetDungeon()->GetGirl(g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

					int selection;

					for(	selection = sel.first();
						selection != -1;
						selection = sel.next()
					) {
						if(g_Girls.IsInvFull(targetGirl))
						{
							g_MessageQue.AddToQue("Her inventory is full", 0);
							break;
						}

						// can player afford the item
						int cost = g_InvManager.GetShopItem(selection)->m_Cost;
						if(!g_Gold.item_cost(cost))
						{
							g_MessageQue.AddToQue("You don't have enough money.", 0);
							break;
						}

						if(g_InvManager.GetShopItem(selection)->m_Badness >= 20)
						{
							// DK: Change default behaviour to equipping an item even if she doesn't like it.
							// (Saves the player some clicking)
							if(g_InvManager.GetShopItem(selection)->m_Type != INVFOOD && g_InvManager.GetShopItem(selection)->m_Type != INVMAKEUP)
							     g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, g_InvManager.BuyShopItem(selection)), false);
						    else
							     g_Girls.AddInv(targetGirl, g_InvManager.BuyShopItem(selection));
							g_InvManager.BuyShopItem(selection);
							g_MessageQue.AddToQue("She doesn't seem happy with the gift.", 0);
							continue;
						}
						int happiness = 0;
						if(cost > 50)
							happiness = cost/50;
						else
							happiness = 1;

						// TODO Add multiple variations of text based on feelings towards player
						g_MessageQue.AddToQue("She is happy with the gift and gives you a big hug and a kiss on the cheek.", 0);

						if(g_InvManager.GetShopItem(selection)->m_Type != INVFOOD && g_InvManager.GetShopItem(selection)->m_Type != INVMAKEUP)
							g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, g_InvManager.BuyShopItem(selection)), false);
						else
							g_Girls.AddInv(targetGirl, g_InvManager.BuyShopItem(selection));
						g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
						g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
						g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
						g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness-1);
						g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);
					}
				}
				else	// player buying from shop
				{
					// buy from shop, taking money from player
					int pos = 0;

					for(int	selection = sel.first();
						selection != -1;
						selection = sel.next()
					) {
						int cost = g_InvManager.GetShopItem(selection)->m_Cost;
						if(!g_Gold.afford(cost))
						{
							g_MessageQue.AddToQue("You don't have enough gold.", 1);
							break;
						}
						sInventoryItem *iipt = g_InvManager.GetShopItem(selection);

						if(!g_Brothels.AddItemToInventory(iipt)) {
							g_MessageQue.AddToQue("Your inventory is full.", 1);
							break;
						}

						g_InvManager.BuyShopItem(selection);
						g_Gold.item_cost(cost);
					}
				}
			}
			else if(rightOwner == 0) // target is player
			{
				if(leftOwner == 1)	// selling to shop
				{
					for(int	selection = sel.first();
						selection != -1;
						selection = sel.next()
					) {
						long gold = (int)((float)g_InvManager.GetShopItem(selection)->m_Cost*0.5f);
						g_Gold.item_sales(gold);
						g_Brothels.m_NumItem[selection]--;
						if(g_Brothels.m_NumItem[selection] == 0)
						{
							g_Brothels.m_Inventory[selection] = 0;
							g_Brothels.m_EquipedItems[selection] = 0;
							g_Brothels.m_NumInventory--;
						}
					}
				}
				else	// giving gift to a girl
				{
					sGirl* targetGirl = 0;
					if(leftOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
						targetGirl = g_Brothels.GetGirl(g_CurrBrothel, g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-2);
					else	// dungeon girl
						targetGirl = (g_Brothels.GetDungeon()->GetGirl(g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

					// take items from player and give to the girl
					int pos = 0;
					int selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, 0, pos);
					while(selection != -1)
					{
						if(g_Girls.IsInvFull(targetGirl))
						{
							g_MessageQue.AddToQue("Her inventory is full", 0);
							break;
						}

						if(g_Brothels.m_Inventory[selection]->m_Badness < 20)
						{
							int happiness = 0;
							if(g_Brothels.m_Inventory[selection]->m_Cost > 50)
								happiness = g_Brothels.m_Inventory[selection]->m_Cost/50;
							else
								happiness = 1;

							// TODO Add multiple variations of text based on feelings towards player
							g_MessageQue.AddToQue("She is happy with the gift and gives you a big hug and a kiss on the cheek.", 0);

							if(g_Brothels.m_Inventory[selection]->m_Type != INVFOOD && g_Brothels.m_Inventory[selection]->m_Type != INVMAKEUP)
								g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, g_Brothels.m_Inventory[selection]), false);
							else
								g_Girls.AddInv(targetGirl, g_Brothels.m_Inventory[selection]);
							g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
							g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
							g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness-1);
							g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);
						}
						else
						{
							if(g_Brothels.m_Inventory[selection]->m_Type != INVFOOD && g_Brothels.m_Inventory[selection]->m_Type != INVMAKEUP)
								g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, g_Brothels.m_Inventory[selection]), false);
							else
							g_Girls.AddInv(targetGirl, g_Brothels.m_Inventory[selection]);
							g_MessageQue.AddToQue("She doesn't seem happy with the gift.", 0);
						}

						g_Brothels.m_NumItem[selection]--;
						if(g_Brothels.m_NumItem[selection] == 0)
						{
							g_Brothels.m_Inventory[selection] = 0;
							g_Brothels.m_EquipedItems[selection] = 0;
							g_Brothels.m_NumInventory--;
						}

						selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, pos+1, pos);
					}
				}
			}
			else	// target is girl
			{
				sGirl* targetGirl = 0;
				if(rightOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
					targetGirl = g_Brothels.GetGirl(g_CurrBrothel, g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSR)-2);
				else // dungeon girl
					targetGirl = (g_Brothels.GetDungeon()->GetGirl(g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSR)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

				if(leftOwner == 0)	// player taking item from the girl
				{
					int pos = 0;
					int selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, 0, pos);
					while(selection != -1)
					{
						// Add the item to players inventory
						if(g_Brothels.AddItemToInventory(targetGirl->m_Inventory[selection]) == false)
						{
							g_MessageQue.AddToQue("Your inventory is full.", 1);
							break;
						}

						if(targetGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
							g_InvManager.Unequip(targetGirl, selection);
						if(targetGirl->m_Inventory[selection]->m_Badness >= 20)	// happy to get rid of bad items
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, 5);
						else	// sad to see good items go
						{
							int happiness = targetGirl->m_Inventory[selection]->m_Cost/50;
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, -happiness);
						}

						// remove the item from the girl
						targetGirl->m_Inventory[selection] = 0;
						targetGirl->m_EquipedItems[selection] = 0;
						targetGirl->m_NumInventory--;

						selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, pos+1, pos);
					}
				}
				else if(leftOwner == 1)	// player seling item straight to the shop
				{
					// take items and transfer to girl, taking money from player
					int pos = 0;
					int selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, 0, pos);
					while(selection != -1)
					{
						if(targetGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
							g_InvManager.Unequip(targetGirl, selection);
						if(targetGirl->m_Inventory[selection]->m_Badness >= 20)	// happy to get rid of bad items
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, 5);
						else	// sad to see good items go
						{
							int happiness = targetGirl->m_Inventory[selection]->m_Cost/50;
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, -happiness);
						}

						// add the gold
						long gold = targetGirl->m_Inventory[selection]->m_Cost / 2;

						g_Gold.item_sales(gold);

						// remove the item
						targetGirl->m_Inventory[selection] = 0;
						targetGirl->m_EquipedItems[selection] = 0;
						targetGirl->m_NumInventory--;

						selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, pos+1, pos);
					}
				}
				else	// player forcing a girl to give to another girl
				{
					sGirl* fromGirl = 0;// ********************
					if(leftOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
						fromGirl = g_Brothels.GetGirl(g_CurrBrothel, g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-2);
					else // dungeon girl
						fromGirl = (g_Brothels.GetDungeon()->GetGirl(g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

					int pos = 0;
					int selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, 0, pos);
					while(selection != -1)
					{
						if(g_Girls.IsInvFull(targetGirl))
						{
							g_MessageQue.AddToQue("Her inventory is full", 0);
							break;
						}

						if(targetGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
							g_InvManager.Unequip(fromGirl, selection);
						if(targetGirl->m_Inventory[selection]->m_Badness >= 20)	// happy to get rid of bad items
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, 5);
						else	// sad to see good items go
						{
							int happiness = targetGirl->m_Inventory[selection]->m_Cost/50;
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, -happiness);
						}

						// add to target Girls inventory
						if(targetGirl->m_Inventory[selection]->m_Badness < 20)
						{
							int happiness = 0;
							if(targetGirl->m_Inventory[selection]->m_Cost > 50)
								happiness = targetGirl->m_Inventory[selection]->m_Cost/50;
							else
								happiness = 1;

							// TODO Add multiple variations of text based on feelings towards player
							g_MessageQue.AddToQue("She is happy with the gift and gives you a big hug and a kiss on the cheek.", 0);

							if(targetGirl->m_Inventory[selection]->m_Type != INVFOOD && targetGirl->m_Inventory[selection]->m_Type != INVMAKEUP)
								g_InvManager.Equip(fromGirl, g_Girls.AddInv(fromGirl, targetGirl->m_Inventory[selection]), false);
							else
								g_Girls.AddInv(fromGirl, targetGirl->m_Inventory[selection]);
							g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
							g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
							g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
							g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness-1);
							g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);
						}
						else
						{
							if(targetGirl->m_Inventory[selection]->m_Type != INVFOOD && targetGirl->m_Inventory[selection]->m_Type != INVMAKEUP)
								g_InvManager.Equip(fromGirl, g_Girls.AddInv(fromGirl, targetGirl->m_Inventory[selection]), false);
							else
							g_Girls.AddInv(fromGirl, targetGirl->m_Inventory[selection]);
							g_MessageQue.AddToQue("She doesn't seem happy with the gift.", 0);
						}

						// remove the item from the girl
						fromGirl->m_Inventory[selection] = 0;
						fromGirl->m_EquipedItems[selection] = 0;
						fromGirl->m_NumInventory--;

						selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR, pos+1, pos);
					}
				}
			}

			// update the item lists
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);
		}
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_ITMOWNERSL))
	{
		g_ItemManagement.ClearListBox(g_interfaceid.LIST_ITMITEMSL);
		int selection = g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL);
		if(selection == rightOwner)
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
		else if(selection != -1)
		{
			std::stringstream ss;
			leftOwner = selection;

			if(leftOwner == 0)	// player items
			{
				for(int i=0; i<300; i++)
				{
					if(g_Brothels.m_Inventory[i])
					{
						ss << g_Brothels.m_Inventory[i]->m_Name;
						   << " ("
						   << g_Brothels.m_NumItem[i]
						   << ")"
						;
					    std::string it = ss.str();

						if(filter == 0)
							g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, it);
						else
						{
							if(filter == 4)
							{
								if(g_Brothels.m_Inventory[i]->m_Type == filter || g_Brothels.m_Inventory[i]->m_Type == 7)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, it);
							}
							else
							{
								if(g_Brothels.m_Inventory[i]->m_Type == filter)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, it);
							}
						}
					}
				}
			}
			else if(leftOwner == 1)	// shop items
			{
				for(int i=0; i<NUM_SHOPITEMS; i++)
				{
					if(g_InvManager.GetShopItem(i))
					{
						if(filter == 0)
							g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, g_InvManager.GetShopItem(i)->m_Name);
						else
						{
							if(filter == 4)
							{
								if(g_InvManager.GetShopItem(i)->m_Type == filter || g_InvManager.GetShopItem(i)->m_Type == 7)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, g_InvManager.GetShopItem(i)->m_Name);
							}
							else
							{
								if(g_InvManager.GetShopItem(i)->m_Type == filter)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, g_InvManager.GetShopItem(i)->m_Name);
							}
						}
					}
				}
			}
			else	// girl items
			{
				sGirl* targetGirl = 0;
				if(leftOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
					targetGirl = g_Brothels.GetGirl(g_CurrBrothel, leftOwner-2);
				else // dungeon girl
					targetGirl = (g_Brothels.GetDungeon()->GetGirl(leftOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

				for(int i=0; i<40; i++)
				{
					if(targetGirl->m_Inventory[i])
					{
						if(filter == 0)
							g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, targetGirl->m_Inventory[i]->m_Name);
						else
						{
							if(filter == 4)
							{
								if(targetGirl->m_Inventory[i]->m_Type == filter || targetGirl->m_Inventory[i]->m_Type == 7)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, targetGirl->m_Inventory[i]->m_Name);
							}
							else
							{
								if(targetGirl->m_Inventory[i]->m_Type == filter)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSL, i, targetGirl->m_Inventory[i]->m_Name);
							}
						}
					}
				}
			}
		}

		g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMITEMSL, leftItem);
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_ITMOWNERSR))
	{
		// good enough place as any to update the cost shown on the screen
	    std::string temp = "PLAYER GOLD: ";
		temp += g_Gold.sval();
		g_ItemManagement.EditTextItem(temp, g_interfaceid.TEXT_ITMPLAYERGOLD);

		g_ItemManagement.ClearListBox(g_interfaceid.LIST_ITMITEMSR);
		int selection = g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSR);
		if(selection == leftOwner)
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);
		else if(selection != -1)
		{
			rightOwner = selection;

			if(rightOwner == 0)	// player items
			{
				for(int i=0; i<300; i++)
				{
					if(g_Brothels.m_Inventory[i])
					{
					    std::string it = g_Brothels.m_Inventory[i]->m_Name;
						it += " (";
						it += toString(g_Brothels.m_NumItem[i]);
						it += ")";
						if(filter == 0)
							g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, it);
						else
						{
							if(filter == 4)
							{
								if(g_Brothels.m_Inventory[i]->m_Type == filter || g_Brothels.m_Inventory[i]->m_Type == 7)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, it);
							}
							else
							{
								if(g_Brothels.m_Inventory[i]->m_Type == filter)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, it);
							}
						}
					}
				}
			}
			else if(rightOwner == 1)	// shop items
			{
				for(int i=0; i<NUM_SHOPITEMS; i++)
				{
					if(g_InvManager.GetShopItem(i))
					{
						if(filter == 0)
							g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, g_InvManager.GetShopItem(i)->m_Name);
						else
						{
							if(filter == 4)
							{
								if(g_InvManager.GetShopItem(i)->m_Type == filter || g_InvManager.GetShopItem(i)->m_Type == 7)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, g_InvManager.GetShopItem(i)->m_Name);
							}
							else
							{
								if(g_InvManager.GetShopItem(i)->m_Type == filter)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, g_InvManager.GetShopItem(i)->m_Name);
							}
						}
					}
				}
			}
			else	// girl items
			{
				sGirl* targetGirl = 0;
				if(rightOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
					targetGirl = g_Brothels.GetGirl(g_CurrBrothel, rightOwner-2);
				else // dungeon girl
					targetGirl = (g_Brothels.GetDungeon()->GetGirl(rightOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

				for(int i=0; i<40; i++)
				{
					if(targetGirl->m_Inventory[i])
					{
						if(filter == 0)
							g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, targetGirl->m_Inventory[i]->m_Name);
						else
						{
							if(filter == 4)
							{
								if(targetGirl->m_Inventory[i]->m_Type == filter || targetGirl->m_Inventory[i]->m_Type == 7)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, targetGirl->m_Inventory[i]->m_Name);
							}
							else
							{
								if(targetGirl->m_Inventory[i]->m_Type == filter)
									g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMITEMSR, i, targetGirl->m_Inventory[i]->m_Name);
							}
						}
					}
				}
			}
		}
		g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMITEMSR, rightItem);
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_ITMITEMSL))
	{
		int selection = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSL);
		if(selection != -1)
		{
		    std::string temp = "Cost: ";
			if(leftOwner == 0)
			{
				temp += toString(g_Brothels.m_Inventory[selection]->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_Brothels.m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += g_Brothels.m_Inventory[selection]->m_Desc;
				g_ItemManagement.EditTextItem(temp, g_interfaceid.TEXT_ITMITEMDESC);

				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, true);
				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, true);
			}
			else if(leftOwner == 1)
			{
				temp += toString(g_InvManager.GetShopItem(selection)->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_InvManager.GetShopItem(selection)->m_Name;
				temp += "\n";
				temp += g_InvManager.GetShopItem(selection)->m_Desc;
				g_ItemManagement.EditTextItem(temp, g_interfaceid.TEXT_ITMITEMDESC);

				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, true);
				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, true);
			}
			else
			{
				sGirl* targetGirl = 0;
				if(leftOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
					targetGirl = g_Brothels.GetGirl(g_CurrBrothel, leftOwner-2);
				else // dungeon girl
					targetGirl = (g_Brothels.GetDungeon()->GetGirl(leftOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

				temp += toString(targetGirl->m_Inventory[selection]->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += targetGirl->m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += targetGirl->m_Inventory[selection]->m_Desc;
				g_ItemManagement.EditTextItem(temp, g_interfaceid.TEXT_ITMITEMDESC);

				if(g_InvManager.IsItemEquipable(targetGirl->m_Inventory[selection]))
				{

					if(targetGirl->m_EquipedItems[selection] == 1)
					{
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, true);
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, false);
					}
					else
					{
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, false);
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, true);
					}
				}
				else
				{
					g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, true);
					g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, true);
				}
			}
		}
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_ITMITEMSR))
	{
		int selection = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR);
		if(selection != -1)
		{
		    std::string temp = "Cost: ";
			if(rightOwner == 0)
			{
				temp += toString(g_Brothels.m_Inventory[selection]->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_Brothels.m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += g_Brothels.m_Inventory[selection]->m_Desc;
				g_ItemManagement.EditTextItem(temp, g_interfaceid.TEXT_ITMITEMDESC);

				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, true);
				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, true);
			}
			else if(rightOwner == 1)
			{
				temp += toString(g_InvManager.GetShopItem(selection)->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_InvManager.GetShopItem(selection)->m_Name;
				temp += "\n";
				temp += g_InvManager.GetShopItem(selection)->m_Desc;
				g_ItemManagement.EditTextItem(temp, g_interfaceid.TEXT_ITMITEMDESC);

				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, true);
				g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, true);
			}
			else
			{
				sGirl* targetGirl = 0;
				if(rightOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
					targetGirl = g_Brothels.GetGirl(g_CurrBrothel, rightOwner-2);
				else // dungeon girl
					targetGirl = (g_Brothels.GetDungeon()->GetGirl(rightOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

				temp += toString(targetGirl->m_Inventory[selection]->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += targetGirl->m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += targetGirl->m_Inventory[selection]->m_Desc;
				g_ItemManagement.EditTextItem(temp, g_interfaceid.TEXT_ITMITEMDESC);

				if(g_InvManager.IsItemEquipable(targetGirl->m_Inventory[selection]))
				{

					if(targetGirl->m_EquipedItems[selection] == 1)
					{
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, true);
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, false);
					}
					else
					{
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, false);
						g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, true);
					}
				}
				else
				{
					g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, true);
					g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, true);
				}
			}
		}
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_ITMFILTER))
	{
		int selection = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMFILTER);
		filter = selection;
		g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
		g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_ITMEQUIP1))
	{
		sGirl* targetGirl = 0;
		if(leftOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, leftOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(leftOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

		int selection = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSL);
		if(selection != -1)
		{
			g_InvManager.Equip(targetGirl, selection, true);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, true);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, false);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);
		}
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_ITMUNEQUIP1))
	{
		sGirl* targetGirl = 0;
		if(leftOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, leftOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(leftOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

		int selection = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSL);
		if(selection != -1)
		{
			g_InvManager.Unequip(targetGirl, selection);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, false);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, true);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);
		}
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_ITMEQUIP2))
	{
		sGirl* targetGirl = 0;
		if(rightOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, rightOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(rightOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;


		int selection = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR);
		if(selection != -1)
		{
			g_InvManager.Equip(targetGirl, selection, true);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, true);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, false);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);
		}
	}
	else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_ITMUNEQUIP2))
	{
		sGirl* targetGirl = 0;
		if(rightOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, rightOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(rightOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

		int selection = g_ItemManagement.GetLastSelectedItemFromList(g_interfaceid.LIST_ITMITEMSR);
		if(selection != -1)
		{
			g_InvManager.Unequip(targetGirl, selection);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, false);
			g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, true);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
			g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);
		}
	}
}

void cInventoryManagementSM::init()
{
    std::string brothel = "Current Brothel: ";
	brothel += g_Brothels.GetName(g_CurrBrothel);
	g_ItemManagement.EditTextItem(brothel, g_interfaceid.TEXT_CURRENTBROTHEL);

	NumDungeonGirls = NumBrothelGirls = leftOwner = 0;
	rightOwner = 1;
	// clear list boxes
	g_ItemManagement.ClearListBox(g_interfaceid.LIST_ITMOWNERSL);
	g_ItemManagement.ClearListBox(g_interfaceid.LIST_ITMOWNERSR);
	g_ItemManagement.ClearListBox(g_interfaceid.LIST_ITMITEMSL);
	g_ItemManagement.ClearListBox(g_interfaceid.LIST_ITMITEMSR);
	g_ItemManagement.ClearListBox(g_interfaceid.LIST_ITMFILTER);

	// setup the filter
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 0, "All");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 1, "Rings");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 2, "Dress");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 3, "Shoes");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 4, "Consumables");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 5, "Necklace");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 6, "Weapon");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 8, "Armor");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 9, "Misc");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMFILTER, 10, "Armband");
	if(filter == -1)
		filter = 0;
	g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMFILTER, filter, false);

	// add shop and player to list
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSL, 0, "Player");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSL, 1, "Shop");

	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSR, 0, "Player");
	g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSR, 1, "Shop");

	// and girls from current brothel to list
	int i=2;
	sGirl* temp = g_Brothels.GetGirl(g_CurrBrothel, 0);
	while(temp)
	{
		if(temp == 0)
			break;
		if(g_AllTogle == true && g_ReturnText == temp->m_Realname)
			rightOwner = i;

		g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSL, i, temp->m_Realname);
		g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSR, i, temp->m_Realname);
		NumBrothelGirls++;
		temp = temp->m_Next;
		i++;
	}

	// add current dungeon girls to list
	sDungeonGirl* temp2 = g_Brothels.GetDungeon()->GetGirl(0);
	while(temp2)
	{
		if(temp2 == 0)
			break;
		if(g_AllTogle == false && g_ReturnText == temp2->m_Girl->m_Realname)
			rightOwner = i;
		g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSL, i, temp2->m_Girl->m_Realname);
		g_ItemManagement.AddToListBox(g_interfaceid.LIST_ITMOWNERSR, i, temp2->m_Girl->m_Realname);
		NumDungeonGirls++;
		temp2 = temp2->m_Next;
		i++;
	}

	g_ReturnText = "";
	g_AllTogle = false;

	g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSL, leftOwner);
	g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMOWNERSR, rightOwner);

	g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMITEMSL, leftItem);
	g_ItemManagement.SetSelectedItemInList(g_interfaceid.LIST_ITMITEMSR, rightItem);

	// disable the equip/unequip buttons
	g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP1, true);
	g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP1, true);
	g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMEQUIP2, true);
	g_ItemManagement.DisableButton(g_interfaceid.BUTTON_ITMUNEQUIP2, true);

	g_InitWin = false;
}

sGirl *InventoryManagementSM::get_girl_from_list(int owner_idx, int list_id)
{
	int idx, girls_in_brothel = g_Brothels.GetNumGirls(g_CurrBrothel);
/*
 *	is the girl a brothel girl?
 *
 *	(I don't understand the +1 in this test..)
 */
	if(owner_idx <= girls_in_brothel + 1) {
/*
 *		or the -2 here...
 */
		idx = g_ItemManagement.GetSelectedItemFromList(list_id) - 2;
		return g_Brothels.GetGirl(g_CurrBrothel, idx);
	}
/*
 *	if she's in the dungeon, it's basically the same
 *	but we need to adjust the index for the brothel girls
 *	ahead of her in the list
 */
	idx = g_ItemManagement.GetSelectedItemFromList(list_id) - 2;
	idx -= girls_in_brothel;
/*
 *	and we need to look her up in the dungeon, of course
 */
	return g_Brothels.GetDungeon()->GetGirl(idx)->m_Girl;
}

void cInventoryManagementSM::take_from_player(int index)
{
/*
 *	decrement the number of items of that type
 */
	g_Brothels.m_NumItem[index]--;
/*
 *	if there are items remaining in the slot
 *	we can postpone freeing up the slot
 */
	if(g_Brothels.m_NumItem[index]) {
		return;
	}
/*
 *	or not, as the case may be
 */
	g_Brothels.m_Inventory[index] = 0;
	g_Brothels.m_EquipedItems[index] = 0;
	g_Brothels.m_NumInventory--;
}

void cInventoryManagementSM::take_from_girl(sGirl *girl, int index)
{
	sInventoryItem *item = girl->m_Inventory[selection];
/*
 *	flag the item as un-equipped
 */
	if(girl->m_EquipedItems[index] == 1) {
		g_InvManager.Unequip(girl, selection);
	}
/*
 *	if it's a bad item, getting rid of it will make her happier
 *	I have a mind to make the amount of happiness depend on the
 *	degree of badness involved.
 *
 *	Losing a good item will make her sad, as well
 */
	if(item->m_Badness >= 20) {
		g_Girls.UpdateStat(girl, STAT_HAPPINESS, item->m_Badness/5);
	}
	else {
		int happiness = item->m_Cost/50;
		g_Girls.UpdateStat(girl, STAT_HAPPINESS, -happiness);
	}
/*
 *	remove the item
 */
	girl->m_Inventory[selection] = 0;
	girl->m_EquipedItems[selection] = 0;
	girl->m_NumInventory--;
}

/*
 * I have a faint urge to try and define some n-dimensional fuzzy matrix
 * handling thingumabob to handle this
 *
 * could do it as a basic neural network ... add results with the
 * set of stats they're most strongly
 *
 * would we need to consider more that hate, love and fear?
 * libido?
 */
string cInventoryManagementSM::nice_item_message(sGirl *girl, sInventoryItem *item)
{
	std::stringstream ss;
/*
 *	if she's obedient enough, she won't let her personal
 *	feelings get in the way
 */
	if(girl->obedience() >= 90 && girl->pchate() > 25) {
/*
 *		if she's high enough level, she can go into
 *		professional mode
 */
		if(girl->level() >= 4) {
			ss << girl->m_Realname << " smiles warmly at you, "
			   << "compliments you on your good taste, "
			   << "humbly accepts your kind gift and "
			   << "thanks you formally, for being so thoughtful.\n\n"
			   << "\"Is there anything further I can do for you, sir?\""
			;	// and if the answer is "yes", he'll get that, too...
			return ss.str();
		}
		return	"Carefully formal, she thanks you for your kind gift "
			"and enquires if there is anything else she can do";
	}
/*
 *	high levels of hate...
 */
	if(girl->pchate() > 75) {
		if(girl->pcfear() >= 75) {
			if(girl->pclove() >= 75) {
				return "She bursts into tears and "
					"refuses to talk to you"
				;
			}
			return	"She accepts your gift in silence, "
				"her face carefully immobile."
			;
		}
		if(girl->pcfear() >= 25) {
			if(girl->pclove() >= 50) {
				return "\"Hummph! I may be a whore "
					"but I'm not THAT easily bought!\""
				;
			}
			return	"She turns her back on you "
				"refusing to even look at your gift."
			;
		}
		return 	"She flings the gift back in your face, "
			"screaming \"Get out! Get out! Get OUT!\""
		;
	}
/*
 *	it's not easy deciding on an order of evaluation here
 */


	return "She is happy with the gift and gives you a big hug and a kiss on the cheek.";
}

void cInventoryManagementSM::give_to_girl(sGirl *girl, sInventoryItem *item)
{
	int happiness = 1;
/*
 *	bad items make her unhappy. Well, actually, they don't.
 *	they make her complain, but there's not happiness hit
 *	this means you can cheer a girl up by repeatedly
 *	giving her bad things and then taking them away again
 *
 *	In the absence of any indication otherwise,
 *	I'm going to consider this a bug, and fix it
 *
 *	also very bad items will upset her more
 */
	if(item->m_Badness >= 20) {
		int slot2 = g_Girls.AddInv(girl, item);
		if(item->m_Type != INVFOOD && item->m_Type != INVMAKEUP)
			g_InvManager.Equip(girl, slot2, false);
		g_MessageQue.AddToQue("She doesn't seem happy with the gift.", 0);
		g_Girls.UpdateStat(girl, STAT_HAPPINESS, item->m_Badness/5);
		return;
	}
/*
 *	work out how much happier this makes her
 */
	if(item->m_Cost > 50) happiness = item->m_Cost/50;
/*
 *	queue some feedback from the girl
 */
	g_MessageQue.AddToQue(nice_item_message(), 0);

	int slot = g_Girls.AddInv(girl, item);
	if(item->m_Type != INVFOOD && item->m_Type != INVMAKEUP) {
		g_InvManager.Equip(girl, slot, false);
	}
	girl->obedience(1);
	girl->happiness(happiness);
	girl->pchate(-2);
	girl->pclove(happiness-1);
	girl->pcfear(-1);
}

void cInventoryManagementSM::item_shift_r()
{
	int l_owners_id = g_interfaceid.LIST_ITMOWNERSL;
	int r_owners_id = g_interfaceid.LIST_ITMOWNERSR;
	cSelector sel(g_ItemManagement, g_interfaceid.LIST_ITMITEMSL);
/*
 *	we're transfering items from left to right
 *	for this to make sense, both sides need a selected target
 */
	if(rightOwner == -1 || leftOwner == -1) {
		return;
	}
/*
 *	get the indices of the items on each side
 */
	leftItem = g_ItemManagement.GetLastSelectedItemFromList(
		g_interfaceid.LIST_ITMITEMSL
	);
	rightItem = g_ItemManagement.GetLastSelectedItemFromList(
		g_interfaceid.LIST_ITMITEMSR
	);

	if(rightOwner == 1)	// target is shop
	{
		sGirl* targetGirl = 0;

		for(int	selection = sel.first(); selection != -1; selection = sel.next()) {
			if(leftOwner > 1)	// taking from a girl and selling to shop
			{
				targetGirl = get_girl_from_list(leftOwner, l_owners_id);
				sInventoryItem *item = targetGirl->m_Inventory[selection];
/*
 *				loop through the selected items
 */
				take_from_girl(targetGirl, selection);
				// add the gold
				g_Gold.item_sales(tariff.item_sale_price(item));
			}
			else	// player selling to shop
			{
				sInventoryItem *item = g_Brothels.m_Inventory[selection];
/*
 *				remove the item from the player inventory
 */
				take_from_player(selection);
/*
 *				give him the cash
 */
				g_Gold.item_sales(item_buy_price(item->m_Cost));
			}
		}
	}
	else if(rightOwner == 0) // target is player
	{
		if(leftOwner == 1)	// buying from shop
		{
			for(int	selection = sel.first();
				selection != -1;
				selection = sel.next()
			) {
				sInventoryItem* item = g_InvManager.GetShopItem(selection);
/*
 *				get the price of this item
 */
				int cost = tariff.item_buy_price(item);
/*
 *				do we have enough gold?
 */
				if(g_Gold.afford(cost) == false) {
					g_MessageQue.AddToQue("You don't have enough gold.", 1);
					break;	// not point in continuing
				}
/*
 *				see if there's room
 */
				int slot = g_Brothels.get_slot_for_item(item);
				if(slot == -1) {
					g_MessageQue.AddToQue("Your inventory is full.", 1);
					break;
				}
/*
 *				add to player inventory using the slot
 */
				add_item_to_inventory(item, slot);
/*
 *				remove the item from the shop list
 */
				g_InvManager.BuyShopItem(selection);
/*
 *				pay the shopkeeper
 */
				g_Gold.item_cost(gold);
			}
		}
		else	// taking from girl
		{
			// take items from girl and give to player
			sGirl* targetGirl = get_girl_from_list(leftOwner, l_owners_id);
/*
 *			loop trought the selected items
 */
			for(int	selection = sel.first();
				selection != -1;
				selection = sel.next()
			) {
				// Add the item to players inventory
				if(g_Brothels.AddItemToInventory(targetGirl->m_Inventory[selection]) == false)
				{
					g_MessageQue.AddToQue("Your inventory is full.", 1);
					break;
				}
				take_from_girl(targetGirl, selection);
			}
		}
	}
	else	// target is girl
	{
		sGirl* targetGirl = get_girl_from_list(rightOwner, r_owners_id);

		if(leftOwner == 0)	// player giving gift to the girl
		{
/*
 *			loop through the selected items
 */
			for(int	selection = sel.first();
				selection != -1;
				selection = sel.next()
			) {
				sInventoryItem* item = g_Brothels.m_Inventory[selection];
/*
 *				check she has room for it
 */
				if(g_Girls.IsInvFull(targetGirl)) {
					g_MessageQue.AddToQue("Her inventory is full", 0);
					break;
				}
/*
 *				take it from the player
 */
				take_from_player(selection);
/*
 *				give to girl
 */
				give_to_girl(item);
			}
		}
		else if(leftOwner == 1)	// player giving gift straight from the shop
		{
			for(int	selection = sel.first();
				selection != -1;
				selection = sel.next()
			) {
				sInventoryItem* item = g_InvManager.GetShopItem(selection);

				if(g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue("Her inventory is full", 0);
					break;
				}

				int cost = tariff.item_buy_price(item);
/*
 *				can player afford the item
 */
				if(!g_Gold.item_cost(cost)) {
					g_MessageQue.AddToQue("You don't have enough money.", 0);
					break;
				}
				give_to_girl()
			}
		}
		else	// player forcing a girl to give to another girl
		{
			sGirl* fromGirl = 0;
			if(leftOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
				fromGirl = g_Brothels.GetGirl(g_CurrBrothel, g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-2);
			else // dungeon girl
				fromGirl = (g_Brothels.GetDungeon()->GetGirl(g_ItemManagement.GetSelectedItemFromList(g_interfaceid.LIST_ITMOWNERSL)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

			int pos = 0;
			int selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSL, 0, pos);
			while(selection != -1)
			{
				if(g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue("Her inventory is full", 0);
					break;
				}

				if(fromGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
					g_InvManager.Unequip(fromGirl, selection);
				if(fromGirl->m_Inventory[selection]->m_Badness >= 20)	// happy to get rid of bad items
					g_Girls.UpdateStat(fromGirl, STAT_HAPPINESS, 5);
				else	// sad to see good items go
				{
					int happiness = fromGirl->m_Inventory[selection]->m_Cost/50;
					g_Girls.UpdateStat(fromGirl, STAT_HAPPINESS, -happiness);
				}

				// add to target Girls inventory
				if(fromGirl->m_Inventory[selection]->m_Badness < 20)
				{
					int happiness = 0;
					if(fromGirl->m_Inventory[selection]->m_Cost > 50)
						happiness = fromGirl->m_Inventory[selection]->m_Cost/50;
					else
						happiness = 1;

					// TODO Add multiple variations of text based on feelings towards player
					g_MessageQue.AddToQue("She is happy with the gift and gives you a big hug and a kiss on the cheek.", 0);

					if(fromGirl->m_Inventory[selection]->m_Type != INVFOOD && fromGirl->m_Inventory[selection]->m_Type != INVMAKEUP)
						g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]), false);
					else
						g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]);
					g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
					g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
					g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness-1);
					g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);
				}
				else
				{
					if(fromGirl->m_Inventory[selection]->m_Type != INVFOOD && fromGirl->m_Inventory[selection]->m_Type != INVMAKEUP)
						g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]), false);
					else
					g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]);
					g_MessageQue.AddToQue("She doesn't seem happy with the gift.", 0);
				}

				// remove the item from the girl
				fromGirl->m_Inventory[selection] = 0;
				fromGirl->m_EquipedItems[selection] = 0;
				fromGirl->m_NumInventory--;

				selection = g_ItemManagement.GetNextSelectedItemFromList(g_interfaceid.LIST_ITMITEMSL, pos+1, pos);
			}
		}
	}

	// update the item lists
}


