
#include "cScreenItemManagement.h"
#include "Helper.hpp"
#include "cBrothel.h"
#include "cWindowManager.h"
#include "cGold.h"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"
#include "sConfig.h"
#include "cMessageBox.h"
#include "cInterfaceEvent.h"
#include "cGirls.h"
#include "cInventory.h"
#include "DirPath.h"

bool cScreenItemManagement::ids_set = false;

static int leftOwner = 0;
static int rightOwner = 1;
static int NumBrothelGirls = 0;
static int NumDungeonGirls = 0;
static int filter = 0;

static int leftItem = -2;
static int rightItem = -2;
static int sel_next_l = -1;
static int sel_next_r = -1;
static std::string sel_name_l = "";
static std::string sel_name_r = "";

static SDL_Color* RarityColor[7];

cScreenItemManagement::cScreenItemManagement()
{
    DirPath dp = DirPath()
        << "Resources"
        << "Interface"
        << "itemmanagement_screen.xml"
    ;
    m_filename = dp.c_str();
    sel_pos_l = -2;
    sel_pos_r = -2;
}

cScreenItemManagement::~cScreenItemManagement()
{
    
}


void cScreenItemManagement::set_ids()
{
	ids_set = true;
	curbrothel_id = get_id("CurrentBrothel");
	back_id = get_id("BackButton");
	shift_l_id = get_id("ShiftLeftButton");
	shift_r_id = get_id("ShiftRightButton");
	gold_id = get_id("PlayerGold");
	desc_id = get_id("ItemDesc");
	owners_l_id = get_id("OwnersLeftList");
	items_l_id = get_id("ItemsLeftList");
	owners_r_id = get_id("OwnersRightList");
	items_r_id = get_id("ItemsRightList");
	filter_id = get_id("FilterList");
	equip_l_id = get_id("EquipLeftButton");
	unequip_l_id = get_id("UnequipLeftButton");
	equip_r_id = get_id("EquipRightButton");
	unequip_r_id = get_id("UnequipRightButton");

	cConfig cfg;
	for(int i=0; i<7; i++)
	{
		RarityColor[i] = cfg.items.rarity_color(i);
	}
}

void cScreenItemManagement::init()
{
	if(!g_InitWin)
		return;

	Focused();
	g_InitWin = false;

////////////////////

    std::string brothel = "Current Brothel: ";
	brothel += g_Brothels.GetName(g_CurrBrothel);
	EditTextItem(brothel, curbrothel_id);

	NumDungeonGirls = NumBrothelGirls = leftOwner = 0;
	rightOwner = 1;
	// clear list boxes
	ClearListBox(owners_l_id);
	ClearListBox(owners_r_id);
	ClearListBox(items_l_id);
	ClearListBox(items_r_id);
	ClearListBox(filter_id);

	// setup the filter
	AddToListBox(filter_id, 0, "All");
	AddToListBox(filter_id, 1, "Rings");
	AddToListBox(filter_id, 2, "Dress");
	AddToListBox(filter_id, 3, "Shoes");
	AddToListBox(filter_id, 4, "Consumables");
	AddToListBox(filter_id, 5, "Necklaces");
	AddToListBox(filter_id, 6, "Large Weapons");
	AddToListBox(filter_id, 11, "Small Weapons");
	AddToListBox(filter_id, 8, "Armor");
	AddToListBox(filter_id, 9, "Misc");
	AddToListBox(filter_id, 10, "Armbands");
	if(filter == -1)
		filter = 0;
	SetSelectedItemInList(filter_id, filter, false);

	// add shop and player to list
	AddToListBox(owners_l_id, 0, "Player");
	AddToListBox(owners_l_id, 1, "Shop");

	AddToListBox(owners_r_id, 0, "Player");
	AddToListBox(owners_r_id, 1, "Shop");

	// and girls from current brothel to list
	int i=2;
	Girl* temp = g_Brothels.GetGirl(g_CurrBrothel, 0);
	while(temp)
	{
		if(temp == nullptr)
			break;
		if(g_AllTogle && selected_girl == temp)
			rightOwner = i;

		AddToListBox(owners_l_id, i, temp->m_Realname);
		AddToListBox(owners_r_id, i, temp->m_Realname);
		NumBrothelGirls++;
		temp = temp->m_Next;
		i++;
	}

	// add current dungeon girls to list
	sDungeonGirl* temp2 = g_Brothels.GetDungeon()->GetGirl(0);
	while(temp2)
	{
		if(temp2 == nullptr)
			break;
		if(g_AllTogle && selected_girl == temp2->m_Girl)
			rightOwner = i;
		AddToListBox(owners_l_id, i, temp2->m_Girl->m_Realname, LISTBOX_RED);
		AddToListBox(owners_r_id, i, temp2->m_Girl->m_Realname, LISTBOX_RED);
		NumDungeonGirls++;
		temp2 = temp2->m_Next;
		i++;
	}

	g_ReturnText = "";
	g_AllTogle = false;

	DisableButton(shift_l_id, true);
	DisableButton(shift_r_id, true);

	SetSelectedItemInList(owners_l_id, leftOwner);
	SetSelectedItemInList(owners_r_id, rightOwner);

	SetSelectedItemInList(items_l_id, leftItem);
	SetSelectedItemInList(items_r_id, rightItem);

	// disable the equip/unequip buttons
	DisableButton(equip_l_id, true);
	DisableButton(unequip_l_id, true);
	DisableButton(equip_r_id, true);
	DisableButton(unequip_r_id, true);
}

void cScreenItemManagement::process()
{
	// we need to make sure the ID variables are set
	if(!ids_set)
		set_ids();

	// set up the window if needed
	init();

	// check to see if there's a button event needing handling
	check_events();
}


void cScreenItemManagement::check_events()
{
	// no events means we can go home
	if(g_InterfaceEvents.GetNumEvents() == 0)
		return;

	// if it's the back button, pop the window off the stack and we're done
	if(g_InterfaceEvents.CheckButton(back_id))
	{
		sel_pos_l = -2;
		sel_pos_r = -2;
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}
	if(g_InterfaceEvents.CheckButton(shift_r_id))
	{
		attempt_transfer(Left);
	}
	if(g_InterfaceEvents.CheckButton(shift_l_id))
	{
		attempt_transfer(Right);
	}
	if(g_InterfaceEvents.CheckListbox(owners_l_id))
	{
		refresh_item_list(Left);
	}
	if(g_InterfaceEvents.CheckListbox(owners_r_id))
	{
		refresh_item_list(Right);
	}
	if(g_InterfaceEvents.CheckListbox(items_l_id))
	{
		int selection = GetLastSelectedItemFromList(items_l_id);

		DisableButton(shift_r_id, (selection < 0) );

		if(selection != -1)
		{
		    std::string temp = "Cost: ";
			if(leftOwner == 0)
			{
				temp = "Value: ";
				temp += toString(g_Brothels.m_Inventory[selection]->m_Cost);
				temp += " gold    ";
				temp += "Sell for: ";
				temp += toString( (g_Brothels.m_Inventory[selection]->m_Cost*0.5f) );
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_Brothels.m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += g_Brothels.m_Inventory[selection]->m_Desc;
				EditTextItem(temp, desc_id);

				DisableButton(equip_l_id, true);
				DisableButton(unequip_l_id, true);
			}
			else if(leftOwner == 1)
			{
				temp += toString(g_InvManager.GetShopItem(selection)->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_InvManager.GetShopItem(selection)->m_Name;
				temp += "\n";
				temp += g_InvManager.GetShopItem(selection)->m_Desc;
				EditTextItem(temp, desc_id);

				DisableButton(equip_l_id, true);
				DisableButton(unequip_l_id, true);
			}
			else
			{
				Girl* targetGirl = nullptr;
				if(leftOwner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
					targetGirl = g_Brothels.GetGirl(g_CurrBrothel, leftOwner-2);
				else // dungeon girl
					targetGirl = (g_Brothels.GetDungeon()->GetGirl(leftOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

				temp = "Value: ";
				temp += toString(targetGirl->m_Inventory[selection]->m_Cost);
				temp += " gold    ";
				temp += "Sell for: ";
				temp += toString( (targetGirl->m_Inventory[selection]->m_Cost*0.5f) );
				temp += " gold\n";
				temp += "Item Name: ";
				temp += targetGirl->m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += targetGirl->m_Inventory[selection]->m_Desc;
				EditTextItem(temp, desc_id);

				if(g_InvManager.IsItemEquipable(targetGirl->m_Inventory[selection]))
				{
					DisableButton(equip_l_id, (targetGirl->m_EquipedItems[selection] == 1));
					DisableButton(unequip_l_id, !(targetGirl->m_EquipedItems[selection] == 1));
				}
				else
				{
					DisableButton(equip_l_id, true);
					DisableButton(unequip_l_id, true);
				}
			}
		}
	}
	if(g_InterfaceEvents.CheckListbox(items_r_id))
	{
		int selection = GetLastSelectedItemFromList(items_r_id);

		DisableButton(shift_l_id, (selection < 0) );

		if(selection != -1)
		{
		    std::string temp = "Cost: ";
			if(rightOwner == 0)
			{
				temp = "Value: ";
				temp += toString(g_Brothels.m_Inventory[selection]->m_Cost);
				temp += " gold    ";
				temp += "Sell for: ";
				temp += toString( (g_Brothels.m_Inventory[selection]->m_Cost*0.5f) );
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_Brothels.m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += g_Brothels.m_Inventory[selection]->m_Desc;
				EditTextItem(temp, desc_id);

				DisableButton(equip_r_id, true);
				DisableButton(unequip_r_id, true);
			}
			else if(rightOwner == 1)
			{
				temp += toString(g_InvManager.GetShopItem(selection)->m_Cost);
				temp += " gold\n";
				temp += "Item Name: ";
				temp += g_InvManager.GetShopItem(selection)->m_Name;
				temp += "\n";
				temp += g_InvManager.GetShopItem(selection)->m_Desc;
				EditTextItem(temp, desc_id);

				DisableButton(equip_r_id, true);
				DisableButton(unequip_r_id, true);
			}
			else
			{
				Girl* targetGirl = nullptr;
				if(rightOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
					targetGirl = g_Brothels.GetGirl(g_CurrBrothel, rightOwner-2);
				else // dungeon girl
					targetGirl = (g_Brothels.GetDungeon()->GetGirl(rightOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

				temp = "Value: ";
				temp += toString(targetGirl->m_Inventory[selection]->m_Cost);
				temp += " gold    ";
				temp += "Sell for: ";
				temp += toString( (targetGirl->m_Inventory[selection]->m_Cost*0.5f) );
				temp += " gold\n";
				temp += "Item Name: ";
				temp += targetGirl->m_Inventory[selection]->m_Name;
				temp += "\n";
				temp += targetGirl->m_Inventory[selection]->m_Desc;
				EditTextItem(temp, desc_id);

				if(g_InvManager.IsItemEquipable(targetGirl->m_Inventory[selection]))
				{
					DisableButton(equip_r_id, (targetGirl->m_EquipedItems[selection] == 1));
					DisableButton(unequip_r_id, !(targetGirl->m_EquipedItems[selection] == 1));
				}
				else
				{
					DisableButton(equip_r_id, true);
					DisableButton(unequip_r_id, true);
				}
			}
		}
	}
	if(g_InterfaceEvents.CheckListbox(filter_id))
	{
		int selection = GetLastSelectedItemFromList(filter_id);
		filter = selection;
		SetSelectedItemInList(owners_l_id, leftOwner);
		SetSelectedItemInList(owners_r_id, rightOwner);
	}
	if(g_InterfaceEvents.CheckButton(equip_l_id))
	{
		Girl* targetGirl = nullptr;
		if(leftOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, leftOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(leftOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

		int selection = GetLastSelectedItemFromList(items_l_id);
		if(selection != -1)
		{
			g_InvManager.Equip(targetGirl, selection, true);
			DisableButton(equip_l_id, true);
			DisableButton(unequip_l_id, false);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
	if(g_InterfaceEvents.CheckButton(unequip_l_id))
	{
		Girl* targetGirl = nullptr;
		if(leftOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, leftOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(leftOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

		int selection = GetLastSelectedItemFromList(items_l_id);
		if(selection != -1)
		{
			g_InvManager.Unequip(targetGirl, selection);
			DisableButton(equip_l_id, false);
			DisableButton(unequip_l_id, true);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
	if(g_InterfaceEvents.CheckButton(equip_r_id))
	{
		Girl* targetGirl = nullptr;
		if(rightOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, rightOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(rightOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;


		int selection = GetLastSelectedItemFromList(items_r_id);
		if(selection != -1)
		{
			g_InvManager.Equip(targetGirl, selection, true);
			DisableButton(equip_r_id, true);
			DisableButton(unequip_r_id, false);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
	if(g_InterfaceEvents.CheckButton(unequip_r_id))
	{
		Girl* targetGirl = nullptr;
		if(rightOwner <= (g_Brothels.GetNumGirls(g_CurrBrothel)+1))	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, rightOwner-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(rightOwner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

		int selection = GetLastSelectedItemFromList(items_r_id);
		if(selection != -1)
		{
			g_InvManager.Unequip(targetGirl, selection);
			DisableButton(equip_r_id, false);
			DisableButton(unequip_r_id, true);
			SetSelectedItemInList(owners_l_id, leftOwner);
			SetSelectedItemInList(owners_r_id, rightOwner);
		}
	}
}


void cScreenItemManagement::refresh_item_list(Side which_list)
{
	// good enough place as any to update the cost shown on the screen
    std::string temp = "PLAYER GOLD: ";
	temp += g_Gold.sval();
	EditTextItem(temp, gold_id);

//	leftOwner = GetSelectedItemFromList(owners_l_id);
//	rightOwner = GetSelectedItemFromList(owners_r_id);

	int item_list, owner_list, *sel_pos, *owner, *other_owner;
    std::string *sel_name;
	if(which_list == Left)
	{
		item_list = items_l_id;
		owner_list = owners_l_id;
		owner = &leftOwner;
		other_owner = &rightOwner;
		sel_pos = &sel_pos_l;
		sel_name = &sel_name_l;
	}
	else
	{
		item_list = items_r_id;
		owner_list = owners_r_id;
		owner = &rightOwner;
		other_owner = &leftOwner;
		sel_pos = &sel_pos_r;
		sel_name = &sel_name_r;
	}

	ClearListBox(item_list);
	int selection = GetSelectedItemFromList(owner_list);
	if(selection == *other_owner)
		SetSelectedItemInList(owner_list, *owner);
	else if(selection != -1)
	{
		*owner = selection;

		if(*owner == 0)	// player items
		{
			for(int i=0; i<MAXNUM_INVENTORY; i++)
			{
				int ItemColor = -1;
				if(g_Brothels.m_Inventory[i])
				{
				    std::string it = g_Brothels.m_Inventory[i]->m_Name;
					if(*sel_name == it)  // if we just transferred this item here, might want to select it
						*sel_pos = i;
					it += " (";
					it += toString(g_Brothels.m_NumItem[i]);
					it += ")";
					int item_type = g_Brothels.m_Inventory[i]->m_Type;
					if(		(filter == 0)  // unfiltered?
						|| (item_type == filter)  // matches filter exactly?
						|| ( (filter == 4) && (item_type == 7) )  // passes "consumable" filter?
						)
					{  // passed the filter, so add it
						if(*sel_name == g_Brothels.m_Inventory[i]->m_Name)
							*sel_pos = i;  // if we just transferred this item here, might want to select it
						AddToListBox(item_list, i, it);
						ItemColor = g_Brothels.m_Inventory[i]->m_Rarity;
					}
				}
				if(ItemColor > -1)
					SetSelectedItemTextColor(item_list, i, RarityColor[ItemColor]);
			}
		}
		else if(*owner == 1)	// shop items
		{
			for(int i=0; i<NUM_SHOPITEMS; i++)
			{
				int ItemColor = -1;
				if(g_InvManager.GetShopItem(i))
				{
					int item_type = g_InvManager.GetShopItem(i)->m_Type;
					if(		(filter == 0)  // unfiltered?
						|| (item_type == filter)  // matches filter exactly?
						|| ( (filter == 4) && (item_type == 7) )  // passes "consumable" filter?
						)
					{  // passed the filter, so add it
						if(*sel_name == g_InvManager.GetShopItem(i)->m_Name)
							*sel_pos = i;  // if we just transferred this item here, might want to select it
						AddToListBox(item_list, i, g_InvManager.GetShopItem(i)->m_Name);
						ItemColor = g_InvManager.GetShopItem(i)->m_Rarity;
					}
				}
				if(ItemColor > -1)
					SetSelectedItemTextColor(item_list, i, RarityColor[ItemColor]);
			}
		}
		else	// girl items
		{
			Girl* targetGirl = nullptr;
			if(*owner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
				targetGirl = g_Brothels.GetGirl(g_CurrBrothel, *owner-2);
			else // dungeon girl
				targetGirl = (g_Brothels.GetDungeon()->GetGirl(*owner-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

			for(int i=0; i<40; i++)
			{
				int ItemColor = -1;
				if(targetGirl->m_Inventory[i])
				{
					int item_type = targetGirl->m_Inventory[i]->m_Type;
					if(		(filter == 0)  // unfiltered?
						|| (item_type == filter)  // matches filter exactly?
						|| ( (filter == 4) && (item_type == 7) )  // passes "consumable" filter?
						)
					{  // passed the filter, so add it
						if(*sel_name == targetGirl->m_Inventory[i]->m_Name)
							*sel_pos = i;  // if we just transferred this item here, might want to select it
						AddToListBox(item_list, i, targetGirl->m_Inventory[i]->m_Name);
						ItemColor = targetGirl->m_Inventory[i]->m_Rarity;
					}
				}
				if(ItemColor > -1)
					SetSelectedItemTextColor(item_list, i, RarityColor[ItemColor]);
			}
		}
	}
	SortListItems(item_list, "");

	*sel_name = "";
	SetSelectedItemInList(item_list, *sel_pos);

	if(which_list == Left)
	{
		DisableButton(equip_r_id, true);
		DisableButton(unequip_r_id, true);
		if (GetSelectedItemFromList(item_list) < 0)
			SetSelectedItemInList(item_list, sel_next_l);
		leftItem = GetSelectedItemFromList(item_list);
		sel_next_l = -2;
	}
	else
	{
		DisableButton(equip_l_id, true);
		DisableButton(unequip_l_id, true);
		if (GetSelectedItemFromList(item_list) < 0)
			SetSelectedItemInList(item_list, sel_next_r);
		rightItem = GetSelectedItemFromList(item_list);
		sel_next_r = -2;
	}

	if ( GetLastSelectedItemFromList(item_list) < 0 )
	{
		EditTextItem("", desc_id);
		DisableButton((which_list == Left) ? shift_r_id : shift_l_id, true);
	}
}

void cScreenItemManagement::attempt_transfer(Side transfer_from)
{
	leftOwner = GetSelectedItemFromList(owners_l_id);
	rightOwner = GetSelectedItemFromList(owners_r_id);

	if((rightOwner == -1 || leftOwner == -1))
		return;

	int source_list, source_owner_list, source_owner, target_owner_list, target_owner;
    std::string *item_name;
	if(transfer_from == Left)
	{
		source_list = items_l_id;
		source_owner_list = owners_l_id;
		source_owner = leftOwner;
		target_owner_list = owners_r_id;
		target_owner = rightOwner;
		item_name = &sel_name_r;
	}
	else
	{
		source_list = items_r_id;
		source_owner_list = owners_r_id;
		source_owner = rightOwner;
		target_owner_list = owners_l_id;
		target_owner = leftOwner;
		item_name = &sel_name_l;
	}

	leftItem = GetLastSelectedItemFromList(items_l_id);
	rightItem = GetLastSelectedItemFromList(items_r_id);
	if(target_owner == 1)	// target is shop
	{
		Girl* targetGirl = nullptr;
		if(source_owner > 1)	// taking from a girl and selling to shop
		{
			if(source_owner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
				targetGirl = g_Brothels.GetGirl(g_CurrBrothel, GetSelectedItemFromList(source_owner_list)-2);
			else	// dungeon girl
				targetGirl = (g_Brothels.GetDungeon()->GetGirl(GetSelectedItemFromList(source_owner_list)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

			// take items and transfer to shop, giving money to player
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while(selection != -1)
			{
				if(targetGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
					g_InvManager.Unequip(targetGirl, selection);
				if(targetGirl->m_Inventory[selection]->m_Badness >= 20)	// happy to get rid of bad items
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, 5);
				else	// sad to see good items go
				{
					int happiness = g_InvManager.HappinessFromItem(targetGirl->m_Inventory[selection]);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, -happiness);
				}

				// add the gold
				long gold = (int)(targetGirl->m_Inventory[selection]->m_Cost*0.5f);
				g_Gold.item_sales(gold);

				// since items sold to shop are simply destroyed, no selection to track here
//				*item_name = targetGirl->m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				// remove the item
				targetGirl->m_Inventory[selection] = nullptr;
				targetGirl->m_EquipedItems[selection] = 0;
				targetGirl->m_NumInventory--;

				selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
			}
		}
		else	// player selling to shop
		{
			// sell to shop, giving money to player
			int pos=0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while(selection != -1)
			{
				// since items sold to shop are simply destroyed, no selection to track here
//				*item_name = g_Brothels.m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				long gold = (int)((float)g_Brothels.m_Inventory[selection]->m_Cost*0.5f);
				g_Gold.item_sales(gold);
				g_Brothels.m_NumItem[selection]--;
				if(g_Brothels.m_NumItem[selection] == 0)
				{
					g_Brothels.m_Inventory[selection] = nullptr;
					g_Brothels.m_EquipedItems[selection] = 0;
					g_Brothels.m_NumInventory--;
				}
				selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
			}
		}
	}
	else if(target_owner == 0) // target is player
	{
		if(source_owner == 1)	// buying from shop
		{
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while(selection != -1)
			{
				int cost = g_InvManager.GetShopItem(selection)->m_Cost;
				if(g_Gold.afford(cost))
				{
					*item_name = g_InvManager.GetShopItem(selection)->m_Name;  // note name of item, for selection tracking in target list

					if(g_Brothels.AddItemToInventory(g_InvManager.GetShopItem(selection)))
					{
						long gold = (int)(g_InvManager.GetShopItem(selection)->m_Cost);
						g_InvManager.BuyShopItem(selection);
						g_Gold.item_cost(gold);
					}
					else
					{
						*item_name = "";
						g_MessageQue.AddToQue("Your inventory is full.", 1);
						break;
					}
				}
				else
				{
					g_MessageQue.AddToQue("You don't have enough gold.", 1);
					break;
				}

				selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
			}
		}
		else	// taking from girl
		{
			// take items from girl and give to player
			Girl* targetGirl = nullptr;
			if(source_owner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
				targetGirl = g_Brothels.GetGirl(g_CurrBrothel, GetSelectedItemFromList(source_owner_list)-2);
			else	// dungeon girl
				targetGirl = (g_Brothels.GetDungeon()->GetGirl(GetSelectedItemFromList(source_owner_list)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
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
					int happiness = g_InvManager.HappinessFromItem(targetGirl->m_Inventory[selection]);
					g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, -happiness);
				}

				*item_name = targetGirl->m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				// remove the item from the girl
				targetGirl->m_Inventory[selection] = nullptr;
				targetGirl->m_EquipedItems[selection] = 0;
				targetGirl->m_NumInventory--;

				selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
			}
		}
	}
	else	// target is girl
	{
		Girl* targetGirl = nullptr;
		if(target_owner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
			targetGirl = g_Brothels.GetGirl(g_CurrBrothel, GetSelectedItemFromList(target_owner_list)-2);
		else // dungeon girl
			targetGirl = (g_Brothels.GetDungeon()->GetGirl(GetSelectedItemFromList(target_owner_list)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

		if(source_owner == 0)	// player giving gift to the girl
		{
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while(selection != -1)
			{
				if(g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue("Her inventory is full", 0);
					break;
				}

				if(g_Brothels.m_Inventory[selection]->m_Badness < 20)
				{
					int happiness = g_InvManager.HappinessFromItem(g_Brothels.m_Inventory[selection]);

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

				*item_name = g_Brothels.m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				g_Brothels.m_NumItem[selection]--;
				if(g_Brothels.m_NumItem[selection] == 0)
				{
					g_Brothels.m_Inventory[selection] = nullptr;
					g_Brothels.m_EquipedItems[selection] = 0;
					g_Brothels.m_NumInventory--;
				}

				selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
			}
		}
		else if(source_owner == 1)	// player giving gift straight from the shop
		{
			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while(selection != -1)
			{
				if(g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue("Her inventory is full", 0);
					break;
				}

				sInventoryItem *ShopItem = g_InvManager.GetShopItem(selection);
				int cost = ShopItem->m_Cost;

				// can player afford the item
				if(!g_Gold.item_cost(cost)) {
					g_MessageQue.AddToQue("You don't have enough money.", 0);
					break;
				}

				*item_name = ShopItem->m_Name;  // note name of item, for selection tracking in target list

				if(ShopItem->m_Badness >= 20)
				{
					if(ShopItem->m_Type != INVFOOD && ShopItem->m_Type != INVMAKEUP)
						g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, ShopItem), false);
					else
						g_Girls.AddInv(targetGirl, ShopItem);

					g_InvManager.BuyShopItem(selection);
					g_MessageQue.AddToQue("She doesn't seem happy with the gift.", 0);
					selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
					continue;
				}

				int happiness = g_InvManager.HappinessFromItem(ShopItem);

				// TODO Add multiple variations of text based on feelings towards player
				g_MessageQue.AddToQue("She is happy with the gift and gives you a big hug and a kiss on the cheek.", 0);

				if(ShopItem->m_Type != INVFOOD && ShopItem->m_Type != INVMAKEUP)	// don't auto equip consumable items
					g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, g_InvManager.BuyShopItem(selection)), false);
				else
					g_Girls.AddInv(targetGirl, g_InvManager.BuyShopItem(selection));
				g_Girls.UpdateStat(targetGirl, STAT_OBEDIENCE, 1);
				g_Girls.UpdateStat(targetGirl, STAT_HAPPINESS, happiness);
				g_Girls.UpdateStat(targetGirl, STAT_PCHATE, -2);
				g_Girls.UpdateStat(targetGirl, STAT_PCLOVE, happiness-1);
				g_Girls.UpdateStat(targetGirl, STAT_PCFEAR, -1);

				selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
			}
		}
		else	// player forcing a girl to give to another girl
		{
			Girl* fromGirl = nullptr;
			if(source_owner <= g_Brothels.GetNumGirls(g_CurrBrothel)+1)	// brothel girl
				fromGirl = g_Brothels.GetGirl(g_CurrBrothel, GetSelectedItemFromList(source_owner_list)-2);
			else // dungeon girl
				fromGirl = (g_Brothels.GetDungeon()->GetGirl(GetSelectedItemFromList(source_owner_list)-(2+g_Brothels.GetNumGirls(g_CurrBrothel))))->m_Girl;

			int pos = 0;
			int selection = GetNextSelectedItemFromList(source_list, 0, pos);
			while(selection != -1)
			{
				if(g_Girls.IsInvFull(targetGirl))
				{
					g_MessageQue.AddToQue("Her inventory is full", 0);
					break;
				}

				int happiness = g_InvManager.HappinessFromItem(fromGirl->m_Inventory[selection]);

				if(fromGirl->m_EquipedItems[selection] == 1)	// unequip item if it is equiped
					g_InvManager.Unequip(fromGirl, selection);

				// add to target Girls inventory
				if(fromGirl->m_Inventory[selection]->m_Badness < 20)
				{
					g_Girls.UpdateStat(fromGirl, STAT_HAPPINESS, -happiness);  // previous owner sad to lose it

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
					g_Girls.UpdateStat(fromGirl, STAT_HAPPINESS, 5);  // previous owner happy to see it go

					if(fromGirl->m_Inventory[selection]->m_Type != INVFOOD && fromGirl->m_Inventory[selection]->m_Type != INVMAKEUP)
						g_InvManager.Equip(targetGirl, g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]), false);
					else
					g_Girls.AddInv(targetGirl, fromGirl->m_Inventory[selection]);
					g_MessageQue.AddToQue("She doesn't seem happy with the gift.", 0);
				}

				*item_name = fromGirl->m_Inventory[selection]->m_Name;  // note name of item, for selection tracking in target list

				// remove the item from the girl
				fromGirl->m_Inventory[selection] = nullptr;
				fromGirl->m_EquipedItems[selection] = 0;
				fromGirl->m_NumInventory--;

				selection = GetNextSelectedItemFromList(source_list, pos+1, pos);
			}
		}
	}

	//set these for re-selection of same items if they're still there
	int pos = 0;
	sel_pos_l = GetNextSelectedItemFromList(items_l_id, 0, pos);
	sel_pos_r = GetNextSelectedItemFromList(items_r_id, 0, pos);
	sel_next_l = GetAfterSelectedItemFromList(items_l_id);
	sel_next_r = GetAfterSelectedItemFromList(items_r_id);

	// update the item lists
	SetSelectedItemInList(source_owner_list, source_owner);
	SetSelectedItemInList(target_owner_list, target_owner);
}

int cScreenItemManagement::multi_left_first()
{
    sel_pos_l = 0;
    return GetNextSelectedItemFromList(items_l_id, 0, sel_pos_l);
}

int cScreenItemManagement::multi_left_next()
{
    return GetNextSelectedItemFromList(items_l_id, sel_pos_l+1, sel_pos_l);
}

int cScreenItemManagement::multi_right_first()
{
    sel_pos_r = 0;
    return GetNextSelectedItemFromList(items_r_id, 0, sel_pos_r);
}

int cScreenItemManagement::multi_right_next()
{
    return GetNextSelectedItemFromList(items_r_id, sel_pos_r+1, sel_pos_r);
}
