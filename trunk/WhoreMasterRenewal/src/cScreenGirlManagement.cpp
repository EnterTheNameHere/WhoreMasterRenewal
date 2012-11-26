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
#include <algorithm>
#include "cBrothel.h"
#include "cScreenGirlManagement.h"
#include "cWindowManager.h"
#include "cGold.h"
#include "cTariff.h"
#include "cJobManager.h"
#include "InterfaceProcesses.h"

extern bool g_InitWin;
extern int g_CurrBrothel;
extern cGold g_Gold;
extern cBrothelManager g_Brothels;
extern cWindowManager g_WinManager;

extern	bool	g_LeftArrow;
extern	bool	g_RightArrow;
extern	bool	g_UpArrow;
extern	bool	g_DownArrow;

static cTariff tariff;
static std::stringstream ss;

static int lastNum = -1;
static int ImageNum = -1;
static bool FireGirl = false;
static bool FreeGirl = false;
static bool SellGirl = false;
static int selection = -1;
static int DayNight = 0;	// 1 is night, 0 is day.

// need to undefine the stupid windows header macro SetJob
#ifdef SetJob
#undef SetJob
#endif
static bool SetJob = false;

extern sGirl *selected_girl;
extern std::vector<int> cycle_girls;
extern int cycle_pos;


bool cScreenGirlManagement::ids_set = false;

void cScreenGirlManagement::set_ids()
{
	ids_set = true;
	back_id = get_id("BackButton");
	curbrothel_id = get_id("CurrentBrothel");
	girllist_id = get_id("GirlList");
	girlimage_id = get_id("GirlImage");
	girldesc_id = get_id("GirlDescription");
	transfer_id = get_id("TransferButton");
	firegirl_id = get_id("FireButton");
	freeslave_id = get_id("FreeSlaveButton");
	viewdetails_id = get_id("ViewDetailsButton");
	sellslave_id = get_id("SellSlaveButton");
	jobtypehead_id = get_id("JobTypeHeader");
	jobtypelist_id = get_id("JobTypeList");
	jobtypedesc_id = get_id("JobTypeDescription");
	jobhead_id = get_id("JobHeader");
	joblist_id = get_id("JobList");
	jobdesc_id = get_id("JobDescription");
	day_id = get_id("DayButton");
	night_id = get_id("NightButton");

	//Set the default sort order for columns, so listbox knows the order in which data will be sent
	SortColumns(girllist_id, m_ListBoxes[girllist_id]->m_ColumnName, m_ListBoxes[girllist_id]->m_ColumnCount);
}

void cScreenGirlManagement::init()
{

	if(SellGirl)
	{
		if(g_ChoiceManager.GetChoice(0) == 0)
		{
/*
 *			We'll need to generate an array of girl IDs for all selected girls,
 *			then sort the array, then step backwards from the last in the array
 *			This is necessary since removing a girl changes the IDs of all after her
 */
			std::vector<int> girl_array;
			GetSelectedGirls(&girl_array);  // get and sort array of girls

			// OK, we have the array, now step through it backwards
			for(int i = girl_array.size(); i --> 0; )
			{
				selected_girl = g_Brothels.GetGirl(g_CurrBrothel, girl_array[i]);
				if(GirlDead(selected_girl) || !selected_girl->is_slave())
					continue;  // if dead or not a slave, can't sell her
				if(selected_girl)
				{
					int cost = tariff.slave_sell_price(selected_girl);

					g_Brothels.RemoveGirl(g_CurrBrothel, selected_girl, false);
					if(selected_girl->m_Realname.compare(selected_girl->m_Name) == 0)
						g_Girls.AddGirl(selected_girl);  // add unique girls back to main pool
					else
					{  // random girls simply get removed from the game
						delete selected_girl;
						selected_girl = 0;
					}

					g_Gold.slave_sales(cost);
					g_InitWin = true;
				}
			}
		}

		g_ChoiceManager.Free();
		SellGirl = false;
	}

	if(FireGirl)
	{
		if(g_ChoiceManager.GetChoice(0) == 0)
		{
			std::vector<int> girl_array;
			GetSelectedGirls(&girl_array);  // get and sort array of girls

			// OK, we have the array, now step through it backwards
			for(int i = girl_array.size(); i --> 0; )
			{
				selected_girl = g_Brothels.GetGirl(g_CurrBrothel, girl_array[i]);
				if(GirlDead(selected_girl) || selected_girl->is_slave())
					continue;  // if dead or a slave, can't fire her
				if(selected_girl)
				{
					g_Brothels.RemoveGirl(g_CurrBrothel, selected_girl, false);
					if(selected_girl->m_Realname.compare(selected_girl->m_Name) == 0)
						g_Girls.AddGirl(selected_girl);  // add unique girls back to main pool
					else
					{  // random girls simply get removed from the game
						delete selected_girl;
						selected_girl = 0;
					}

					g_InitWin = true;
				}
			}
		}
		g_ChoiceManager.Free();
		FireGirl = false;
	}

	if(FreeGirl)
	{
		if(g_ChoiceManager.GetChoice(0) == 0)
		{
			std::vector<int> girl_array;
			GetSelectedGirls(&girl_array);  // get and sort array of girls

			// OK, we have the array, now step through it backwards
			for(int i = girl_array.size(); i --> 0; )
			{
				selected_girl = g_Brothels.GetGirl(g_CurrBrothel, girl_array[i]);
				if(GirlDead(selected_girl) || !selected_girl->is_slave())
					continue;  // if dead or not a slave, can't free her
				if(selected_girl)
				{
					selected_girl->m_States&=~(1<<STATUS_SLAVE);
					g_Brothels.GetPlayer()->disposition(5);
					//g_Brothels.GetPlayer()->m_Disposition += 5;
					//if(g_Brothels.GetPlayer()->m_Disposition > 100)
					//	g_Brothels.GetPlayer()->m_Disposition = 100;
					g_Girls.UpdateStat(selected_girl, STAT_PCLOVE, 10);
					g_Girls.UpdateStat(selected_girl, STAT_PCFEAR, -20);
					g_Girls.UpdateStat(selected_girl, STAT_PCHATE, -25);
					g_Girls.UpdateStat(selected_girl, STAT_OBEDIENCE, 10);
					g_Girls.UpdateStat(selected_girl, STAT_HAPPINESS, 70);
					selected_girl->m_AccLevel = 1;
					selected_girl->m_Stats[STAT_HOUSE] = 60;
					g_InitWin = true;
				}
			}
		}

		g_ChoiceManager.Free();
		FreeGirl = false;
	}


	if(!g_InitWin)
		return;

	Focused();
	g_InitWin = false;

////////////////////

	selection = GetSelectedItemFromList(girllist_id);

    std::string brothel = "Current Brothel: ";
	brothel += g_Brothels.GetName(g_CurrBrothel);
	EditTextItem(brothel, curbrothel_id);

	//selected_girl = 0;

	// clear the lists
	ClearListBox(girllist_id);
	ClearListBox(jobtypelist_id);

	// add the job filters
//	for(int i=0; i<NUMJOBTYPES; i++)  // loop through all job types
	for(unsigned int i=0; i<JOBFILTER_MOVIESTUDIO; i++)  // temporary limit to job types shown
	{
		AddToListBox(jobtypelist_id, i, g_Brothels.m_JobManager.JobFilterName[i]);
	}
	SetSelectedItemInList(jobtypelist_id, JOBFILTER_GENERAL);

	//get a list of all the column names, so we can find which data goes in that column
	std::vector<std::string> columnNames;
	m_ListBoxes[girllist_id]->GetColumnNames(columnNames);
	int numColumns = columnNames.size();
	std::string* Data = new std::string[numColumns];

	// Add girls to list
	for(int i=0; i<g_Brothels.GetNumGirls(g_CurrBrothel); i++)
	{
		sGirl* gir = g_Brothels.GetGirl(g_CurrBrothel, i);
		if (selected_girl == gir)
			selection = i;

		unsigned int item_color = LISTBOX_BLUE;
		if(g_Girls.GetStat(gir, STAT_HEALTH) <= 30 || g_Girls.GetStat(gir, STAT_TIREDNESS) >= 80 || g_Girls.GetStat(gir, STAT_HAPPINESS) <= 30)
			item_color = LISTBOX_RED;

		gir->OutputGirlRow(Data, columnNames);
		AddToListBox(girllist_id, i, Data, numColumns, item_color);
	}
	delete [] Data;

	DisableButton(firegirl_id, true);
	DisableButton(freeslave_id, true);
	DisableButton(sellslave_id, true);
	DisableButton(viewdetails_id, true);

	lastNum = -1;
	g_InitWin = false;

	if(selection >= 0)
	{
		while(selection > GetListBoxSize(girllist_id) && selection != -1)
			selection--;
	}
	if(selection >= 0)
		SetSelectedItemInList(girllist_id, selection);
	else
		SetSelectedItemInList(girllist_id, 0);

	DisableButton(day_id, (DayNight == 0));
	DisableButton(night_id, (DayNight != 0));

	update_image();
}

void cScreenGirlManagement::process()
{
	// we need to make sure the ID variables are set
	if(!ids_set)
		set_ids();

	// handle arrow keys
 	if(check_keys())
		return;

	// set up the window if needed
	init();

	// check to see if there's a button event needing handling
	check_events();
}


bool cScreenGirlManagement::check_keys()
{
	if(g_UpArrow) {
		selection = ArrowUpListBox(girllist_id);
		g_UpArrow = false;
		return true;
	}
	if(g_DownArrow) {
		selection = ArrowDownListBox(girllist_id);
		g_DownArrow = false;
		return true;
	}
	return false;
}

void cScreenGirlManagement::update_image()
{
	// Draw a girls profile picture and description when selected
	if(selected_girl)
	{
		bool Rand = false;
		if(lastNum != selection)
		{
		    std::string text = selected_girl->m_Desc;
			text += "\n\n";
			text += g_Girls.GetGirlMood(selected_girl);
			EditTextItem(text, girldesc_id);
			Rand = true;
			lastNum = selection;
		}

		SetImage(girlimage_id, g_Girls.GetImageSurface(selected_girl, IMGTYPE_PROFILE, Rand, ImageNum));
		if(g_Girls.IsAnimatedSurface(selected_girl, IMGTYPE_PROFILE, ImageNum))
			SetImage(girlimage_id, g_Girls.GetAnimatedSurface(selected_girl, IMGTYPE_PROFILE, ImageNum));

		HideImage(girlimage_id, false);
	}
	else
	{
		selection = lastNum = -1;
		EditTextItem("No Girl Selected", girldesc_id);
		HideImage(girlimage_id, true);
	}
}

void cScreenGirlManagement::check_events()
{
	// no events means we can go home
	if(g_InterfaceEvents.GetNumEvents() == 0)
		return;

	// if it's the back button, pop the window off the stack and we're done
	if(g_InterfaceEvents.CheckButton(back_id)) {
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}
	if(g_InterfaceEvents.CheckButton(viewdetails_id))
	{
		ViewSelectedGirl();
	}
	if(g_InterfaceEvents.CheckButton(day_id))
	{
		DisableButton(day_id, true);
		DisableButton(night_id, false);
		DayNight = 0;
		RefreshSelectedJobType();
	}
	if(g_InterfaceEvents.CheckButton(night_id))
	{
		DisableButton(day_id, false);
		DisableButton(night_id, true);
		DayNight = 1;
		RefreshSelectedJobType();
	}
	if(g_InterfaceEvents.CheckListbox(jobtypelist_id))
	{
		selection = GetSelectedItemFromList(jobtypelist_id);

		if (selection == -1)
			EditTextItem("Nothing Selected", jobtypedesc_id);
		else
		{
			// populate Jobs listbox with jobs in the selected category
			RefreshJobList();
			EditTextItem(g_Brothels.m_JobManager.JobFilterDescription[selection], jobtypedesc_id);
		}
	}
	if(g_InterfaceEvents.CheckListbox(joblist_id))
	{
		selection = GetSelectedItemFromList(joblist_id);
		if(selection != -1)
		{
			// first handle the descriptions
			EditTextItem(g_Brothels.m_JobManager.JobDescription[selection], jobdesc_id);

			// Now assign the job to all the selected girls
			int pos = 0;
			int GSelection = GetNextSelectedItemFromList(girllist_id, 0, pos);
			while(GSelection != -1)
			{
				selected_girl = g_Brothels.GetGirl(g_CurrBrothel, GSelection);
				if(selected_girl)
				{
					bool day = (DayNight == 0) ? true : false;
					int old_job = (day) ? selected_girl->m_DayJob : selected_girl->m_NightJob;

					// handle special job requirements and assign
					// if HandleSpecialJobs returns true, the job assignment was modified or cancelled
					if(g_Brothels.m_JobManager.HandleSpecialJobs(g_CurrBrothel, selected_girl, selection, old_job, day))
					{
						selection = (day) ? selected_girl->m_DayJob : selected_girl->m_NightJob;
						SetSelectedItemInList(joblist_id, selection, false);
					}

					if (old_job != selection)
					{
						// update the girl's listing to reflect the job change
						ss.str("");
						ss << g_Brothels.m_JobManager.JobName[(int)selected_girl->m_DayJob];
						SetSelectedItemColumnText(girllist_id, GSelection, ss.str(), 5);
						ss.str("");
						ss << g_Brothels.m_JobManager.JobName[(int)selected_girl->m_NightJob];
						SetSelectedItemColumnText(girllist_id, GSelection, ss.str(), 6);

						// refresh job worker counts for former job and current job
						SetSelectedItemText(joblist_id, old_job, g_Brothels.m_JobManager.JobDescriptionCount(old_job, g_CurrBrothel, day));
						SetSelectedItemText(joblist_id, selection, g_Brothels.m_JobManager.JobDescriptionCount(selection, g_CurrBrothel, day));
					}
				}
				GSelection = GetNextSelectedItemFromList(girllist_id, pos+1, pos);
			}
		}
		else
			EditTextItem("Nothing Selected", jobdesc_id);
	}
	if(g_InterfaceEvents.CheckListbox(girllist_id))
	{
		selection = GetSelectedItemFromList(girllist_id);
		if(selection != -1)
		{
			selected_girl = g_Brothels.GetGirl(g_CurrBrothel, selection);
			//// If double-clicked, try to bring up girl details
			if(ListDoubleClicked(girllist_id))
				ViewSelectedGirl();
			////
			if(selected_girl->m_States&(1<<STATUS_SLAVE))
			{
				DisableButton(firegirl_id, true);
				DisableButton(freeslave_id, false);
				DisableButton(sellslave_id, false);
			}
			else
			{
				DisableButton(firegirl_id, false);
				DisableButton(freeslave_id, true);
				DisableButton(sellslave_id, true);

			}
			DisableButton(viewdetails_id, false);
			RefreshSelectedJobType();
		}
		else
		{
			DisableButton(firegirl_id, true);
			DisableButton(freeslave_id, true);
			DisableButton(sellslave_id, true);
			DisableButton(viewdetails_id, true);
			selected_girl = 0;
			selection = -1;
		}
		lastNum = -2;
		update_image();
		return;
	}
	if(g_InterfaceEvents.CheckButton(transfer_id))
	{
		g_InitWin = true;
		g_WinManager.Push(TransferGirls, &g_TransferGirls);
		return;
	}
	if(g_InterfaceEvents.CheckButton(firegirl_id))
	{
		if(selected_girl)
		{
			if(IsMultiSelected(girllist_id))
			{  // multiple girls selected
				g_MessageQue.AddToQue("Are you sure you wish to fire the selected girls?", 0);
				g_ChoiceManager.CreateChoiceBox(224, 112, 352, 384, 0, 2, 32, strlen("Keep Them"));
				g_ChoiceManager.AddChoice(0, "Fire Them", 0);
				g_ChoiceManager.AddChoice(0, "Keep Them", 1);
				g_ChoiceManager.SetActive(0);
				FireGirl = true;
			}
			else  // only one girl selected
			{
				if(GirlDead(selected_girl))
					return;
				g_MessageQue.AddToQue("Are you sure you wish to fire the selected girl?", 0);
				g_ChoiceManager.CreateChoiceBox(224, 112, 352, 384, 0, 2, 32, strlen("Keep Her"));
				g_ChoiceManager.AddChoice(0, "Fire Her", 0);
				g_ChoiceManager.AddChoice(0, "Keep Her", 1);
				g_ChoiceManager.SetActive(0);
				FireGirl = true;
			}
		}
		return;
	}
	if(g_InterfaceEvents.CheckButton(freeslave_id))
	{
		if(selected_girl)
		{
			if(IsMultiSelected(girllist_id))
			{  // multiple girls selected
				g_MessageQue.AddToQue("Are you sure you wish to give these girls their freedom?", 0);
				g_ChoiceManager.CreateChoiceBox(224, 112, 352, 384, 0, 2, 32, strlen("Keep as a slaves"));
				g_ChoiceManager.AddChoice(0, "Grant Freedom", 0);
				g_ChoiceManager.AddChoice(0, "Keep as a slaves", 1);
				g_ChoiceManager.SetActive(0);
				FreeGirl = true;
			}
			else  // only one girl selected
			{
				if(GirlDead(selected_girl))
					return;
				g_MessageQue.AddToQue("Are you sure you wish to give this girl her freedom?", 0);
				g_ChoiceManager.CreateChoiceBox(224, 112, 352, 384, 0, 2, 32, strlen("Keep as a slave"));
				g_ChoiceManager.AddChoice(0, "Grant Freedom", 0);
				g_ChoiceManager.AddChoice(0, "Keep as a slave", 1);
				g_ChoiceManager.SetActive(0);
				FreeGirl = true;
			}
		}
		return;
	}
	if(g_InterfaceEvents.CheckButton(sellslave_id))
	{
		if(selected_girl)
		{
			std::stringstream ss;

			if(IsMultiSelected(girllist_id))
			{  // multiple girls selected
				int value = 0, pos = 0;
				int GSelection = GetNextSelectedItemFromList(girllist_id, 0, pos);
				while(GSelection != -1)
				{
					selected_girl = g_Brothels.GetGirl(g_CurrBrothel, GSelection);
					GSelection = GetNextSelectedItemFromList(girllist_id, pos+1, pos);
					if(GirlDead(selected_girl) || !selected_girl->is_slave())
						continue;  // dead and non-slave girls don't count
					if(selected_girl)
						value += tariff.slave_sell_price(selected_girl);
				}

				ss.str("");
				ss << "You can sell these girls for " << value << " gold. Do you wish to proceed?";
				g_MessageQue.AddToQue(ss.str(), 0);
				g_ChoiceManager.CreateChoiceBox(224, 112, 352, 384, 0, 2, 32, strlen("Keep Girls"));
				g_ChoiceManager.AddChoice(0, "Sell Girls", 0);
				g_ChoiceManager.AddChoice(0, "Keep Girls", 1);
				g_ChoiceManager.SetActive(0);
				SellGirl = true;
			}
			else  // only one girl selected
			{
				if(GirlDead(selected_girl))
					return;

				// Tell the player the price
				ss.str("");
				ss << "You can sell this girl for "
				   << tariff.slave_sell_price(selected_girl)
				   << " gold. Do you wish to proceed?"
				;
				g_MessageQue.AddToQue(ss.str(), 0);

				// ask if he wants to do it
				g_ChoiceManager.CreateChoiceBox(224, 112, 352, 384, 0, 2, 32, strlen("Keep Girl"));
				g_ChoiceManager.AddChoice(0, "Sell Girl", 0);
				g_ChoiceManager.AddChoice(0, "Keep Girl", 1);
				g_ChoiceManager.SetActive(0);
				SellGirl = true;
			}
		}
	}
}


bool cScreenGirlManagement::GirlDead(sGirl *dgirl)
{
	if(g_Girls.GetStat(dgirl, STAT_HEALTH) <= 0)
	{
		g_MessageQue.AddToQue("This girl is dead. She isn't going to work anymore and her body will be removed by the end of the week.", 1);
		return true;
	}
	else
		return false;
}

void cScreenGirlManagement::RefreshSelectedJobType()
{
	selection = GetSelectedItemFromList(girllist_id);
	if(selection < 0)
		return;

	selected_girl = g_Brothels.GetGirl(g_CurrBrothel, selection);

	u_int job = (DayNight == 0) ? selected_girl->m_DayJob : selected_girl->m_NightJob;

	// set the job filter
	int jobtype = 0;
	for(unsigned int i=0; i<NUMJOBTYPES; i++)
	{
		if (job >= g_Brothels.m_JobManager.JobFilterIndex[i] && job < g_Brothels.m_JobManager.JobFilterIndex[i+1])
			jobtype = i;
	}
	SetSelectedItemInList(jobtypelist_id, jobtype);

	SetJob = true;
}

void cScreenGirlManagement::RefreshJobList()
{
	ClearListBox(joblist_id);
	int job_filter = GetSelectedItemFromList(jobtypelist_id);

	if (job_filter == -1)
		return;

    std::string text = "";
	bool day = (DayNight == 0) ? true : false;

	// populate Jobs listbox with jobs in the selected category
	for(unsigned int i=g_Brothels.m_JobManager.JobFilterIndex[job_filter]; i<g_Brothels.m_JobManager.JobFilterIndex[job_filter+1]; i++)
	{
		if (g_Brothels.m_JobManager.JobName[i] == "")
			continue;
		text = g_Brothels.m_JobManager.JobDescriptionCount(i, g_CurrBrothel, day);
		AddToListBox(joblist_id, i, text);
	}

//	if (SetJob)
//	{
//		SetJob = false;
		// set the job
		if(selected_girl)
		{
			int sel_job = (DayNight == 0) ? (int)selected_girl->m_DayJob : (int)selected_girl->m_NightJob;
			SetSelectedItemInList(joblist_id, sel_job, false);
			EditTextItem(g_Brothels.m_JobManager.JobDescription[sel_job], jobdesc_id);
		}
//	}
}


void cScreenGirlManagement::GetSelectedGirls(std::vector<int> *girl_array)
{  // take passed vector and fill it with sorted list of selected girl IDs
	int pos = 0;
	int GSelection = GetNextSelectedItemFromList(girllist_id, 0, pos);
	while(GSelection != -1)
	{
		girl_array->push_back(GSelection);
		GSelection = GetNextSelectedItemFromList(girllist_id, pos+1, pos);
	}
	sort(girl_array->begin(), girl_array->end());
}

void cScreenGirlManagement::ViewSelectedGirl()
{
	if(selected_girl)
	{
		if(GirlDead(selected_girl))
			return;

		//load up the cycle_girls vector with the ordered list of girl IDs
		FillSortedIDList(girllist_id, &cycle_girls, &cycle_pos);
		for(int i = cycle_girls.size(); i --> 0; )
		{  // no viewing dead girls
			if(g_Brothels.GetGirl(g_CurrBrothel, cycle_girls[i])->health() <= 0)
				cycle_girls.erase(cycle_girls.begin()+i);
		}

		g_InitWin = true;
		g_WinManager.push("Girl Details");
		return;
	}
}
