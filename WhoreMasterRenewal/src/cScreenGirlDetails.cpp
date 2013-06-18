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

#include "cScreenGirlDetails.h"
#include "cBrothel.h"
#include "cWindowManager.h"
#include "cGold.h"
#include "cTariff.h"
#include "cJobManager.h"
#include "InterfaceProcesses.h"
#include "InterfaceGlobals.h"
#include "cGetStringScreenManager.h"
#include "cGangs.h"
#include "cScriptManager.h"
#include "cMessageBox.h"
#include "cInterfaceEvent.h"
#include "cTraits.h"
#include "cGirls.h"
#include "CLog.h"
#include "DirPath.h"

#include <algorithm>

namespace WhoreMasterRenewal
{

static cTariff tariff;
static std::stringstream ss; /// @todo Get rid of this static variable

static int ImageNum = -1;
static int DetailLevel = 0;
static int DayNight = 0;

static bool SetJob = true;

bool cScreenGirlDetails::ids_set = false;

cScreenGirlDetails::cScreenGirlDetails() : cInterfaceWindowXML()
{
    DirPath dp = DirPath()
        << "Resources"
        << "Interface"
        << "girl_details_screen.xml"
    ;
    m_filename = dp.c_str();
}

cScreenGirlDetails::~cScreenGirlDetails()
{
    
}


void cScreenGirlDetails::set_ids()
{
	ids_set = true;
	back_id = get_id("BackButton");
	girlname_id = get_id("GirlName");
	girldesc_id = get_id("GirlDescription");
	girlimage_id = get_id("GirlImage");
	more_id = get_id("MoreButton");
	antipreg_id = get_id("UseAntiPregToggle");
	prev_id = get_id("PrevButton");
	next_id = get_id("NextButton");
	accomup_id = get_id("AccomUpButton");
	accomdown_id = get_id("AccomDownButton");
	inventory_id = get_id("InventoryButton");
	senddungeon_id = get_id("SendDungeonButton");
	reldungeon_id = get_id("ReleaseDungeonButton");
	interact_id = get_id("InteractButton");
	takegold_id = get_id("TakeGoldButton");
	houseperc_id = get_id("HousePercSlider");
	housepercval_id = get_id("HousePercValue");
	gallery_id = get_id("GalleryButton");
	jobtypehead_id = get_id("JobTypeHeader");
	jobtypelist_id = get_id("JobTypeList");
	jobhead_id = get_id("JobHeader");
	joblist_id = get_id("JobList");
	day_id = get_id("DayButton");
	night_id = get_id("NightButton");
	traithead_id = get_id("TraitHeader");
	traitlist_id = get_id("TraitList");
	traitdesc_id = get_id("TraitDescription");
}

void cScreenGirlDetails::Free()
{
	//free up everything else
	cInterfaceWindow::Free();
}

void cScreenGirlDetails::init()
{

	if(selected_girl == nullptr)
	{
		g_WinManager.Pop();
		g_InitWin = true;
		g_LogFile.write("ERROR - girl details screen, selected_girl is null");
/*
 *		adding this because the game will crash if we
 *		go past this point with a null girl
 *
 *		Now as to why it was null in the first place ...
 *		-- doc
 */
 		return;
	}


	if(!g_InitWin)
		return;

	Focused();
	g_InitWin = false;

////////////////////
	if(selected_girl->health() <= 0)
	{
		selected_girl = remove_selected_girl();
		if(selected_girl == nullptr)
		{
			g_WinManager.Pop();
			g_InitWin = true;
	 		return;
		}
	}

	u_int job = (DayNight == 0) ? selected_girl->m_DayJob : selected_girl->m_NightJob;
	SetJob = true;

	EditTextItem(selected_girl->m_Realname, girlname_id);

    std::string detail = (DetailLevel == 0) ? g_Girls.GetDetailsString(selected_girl) : g_Girls.GetMoreDetailsString(selected_girl);
	EditTextItem(detail, girldesc_id);

	if(selected_girl)
	{
		SetImage(girlimage_id, g_Girls.GetImageSurface(selected_girl, IMGTYPE_PROFILE, true, ImageNum));
		if(g_Girls.IsAnimatedSurface(selected_girl, IMGTYPE_PROFILE, ImageNum))
			SetImage(girlimage_id, g_Girls.GetAnimatedSurface(selected_girl, IMGTYPE_PROFILE, ImageNum));
	}

	SliderRange(houseperc_id, 0, 100, g_Girls.GetStat(selected_girl, STAT_HOUSE), 10);
	ss.str("");
	ss << "House Percentage: " << SliderValue(houseperc_id) << "%";
	EditTextItem(ss.str(), housepercval_id);

	ClearListBox(jobtypelist_id);

	DisableButton(interact_id, (g_TalkCount <= 0));
	DisableButton(takegold_id, (selected_girl->m_Money <= 0));
	SetCheckBox(antipreg_id, (selected_girl->m_UseAntiPreg));

	bool InDungeon = (selected_girl->m_DayJob == JOB_INDUNGEON);
	DisableButton(reldungeon_id, !InDungeon);
	DisableButton(senddungeon_id, InDungeon);

	if(!InDungeon)
	{  // if not in dungeon, set up job lists
		// add the job filters
//		for(int i=0; i<NUMJOBTYPES; i++)  // loop through all job types
		for(unsigned int i=0; i<JOBFILTER_MOVIESTUDIO; i++)  // temporary limit to job types shown
		{
			AddToListBox(jobtypelist_id, i, g_Brothels.m_JobManager.JobFilterName[i]);
		}

		// set the job filter
		int jobtype = 0;
		for(unsigned int i=0; i<NUMJOBTYPES; i++)
		{
			if (job >= g_Brothels.m_JobManager.JobFilterIndex[i] && job < g_Brothels.m_JobManager.JobFilterIndex[i+1])
			{
				jobtype = i;
				break;
			}
		}
		SetSelectedItemInList(jobtypelist_id, jobtype);
		RefreshJobList();

		HideButton(day_id, false);
		HideButton(night_id, false);
		DisableButton(day_id, (DayNight == 0));
		DisableButton(night_id, (DayNight != 0));
	}
	else
	{  // if in dungeon, effectively disable job lists
		ClearListBox(joblist_id);
		AddToListBox(jobtypelist_id, -1, "Languish in dungeon");
		HideButton(day_id, true);
		HideButton(night_id, true);
	}

	ClearListBox(traitlist_id);
	for(int i=0; i<30; i++)
	{
		if(selected_girl->m_Traits[i])
			AddToListBox(traitlist_id, i, selected_girl->m_Traits[i]->m_Name);
	}
	EditTextItem("", traitdesc_id);
}


void cScreenGirlDetails::process()
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


bool cScreenGirlDetails::check_keys()
{
	if(g_LeftArrow)
	{
		g_LeftArrow = false;
		PrevGirl();
		return true;
	}
	if(g_RightArrow)
	{
		g_RightArrow = false;
		NextGirl();
		return true;
	}
	return false;
}


void cScreenGirlDetails::check_events()
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
	if(g_InterfaceEvents.CheckSlider(houseperc_id))
	{
		g_Girls.SetStat(selected_girl, STAT_HOUSE, SliderValue(houseperc_id));
		ss.str("");
		ss << "House Percentage: " << SliderValue(houseperc_id) << "%";
		EditTextItem(ss.str(), housepercval_id);
		// Rebelliousness might have changed, so update details
		if(DetailLevel == 0)
		{
		    std::string detail = g_Girls.GetDetailsString(selected_girl);
			EditTextItem(detail, girldesc_id);
		}

		return;
	}
	if(g_InterfaceEvents.CheckButton(more_id))
	{
		if(DetailLevel == 0)
		{
			DetailLevel = 1;
			EditTextItem(g_Girls.GetMoreDetailsString(selected_girl), girldesc_id);
		}
		else
		{
			DetailLevel = 0;
			EditTextItem(g_Girls.GetDetailsString(selected_girl), girldesc_id);
		}
	}
	if(g_InterfaceEvents.CheckButton(day_id))
	{
		DisableButton(day_id, true);
		DisableButton(night_id, false);
		g_InitWin = true;
		DayNight = 0;
	}
	if(g_InterfaceEvents.CheckButton(night_id))
	{
		DisableButton(day_id, false);
		DisableButton(night_id, true);
		g_InitWin = true;
		DayNight = 1;
	}
	if(g_InterfaceEvents.CheckCheckbox(antipreg_id))
	{
		selected_girl->m_UseAntiPreg = (IsCheckboxOn(antipreg_id));
	}
	if(g_InterfaceEvents.CheckListbox(traitlist_id))
	{
		int selection = GetLastSelectedItemFromList(traitlist_id);
		if(selection != -1)
			EditTextItem(selected_girl->m_Traits[selection]->m_Desc, traitdesc_id);
		else
			EditTextItem("", traitdesc_id);
	}
	if(g_InterfaceEvents.CheckListbox(jobtypelist_id))
	{
		SetJob = true;
		RefreshJobList();
	}
	if(g_InterfaceEvents.CheckListbox(joblist_id))
	{
		int selection = GetSelectedItemFromList(joblist_id);
		if(selection != -1)
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

			// refresh job worker counts for former job and current job
			if (old_job != selection)
			{
				SetSelectedItemText(joblist_id, old_job, g_Brothels.m_JobManager.JobDescriptionCount(old_job, g_CurrBrothel, day));
				SetSelectedItemText(joblist_id, selection, g_Brothels.m_JobManager.JobDescriptionCount(selection, g_CurrBrothel, day));
			}
		}
	}
	if(g_InterfaceEvents.CheckButton(inventory_id))
	{
		if(selected_girl)
		{
			if(GirlDead(selected_girl))
				return;

			g_InitWin = true;
			g_AllTogle = true;
			g_WinManager.push("Item Management");
			return;
		}
	}
	if(g_InterfaceEvents.CheckButton(gallery_id))
	{
		g_WinManager.Push(Gallery, &g_Gallery);
		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(accomup_id))
	{
		if(selected_girl->m_AccLevel+1 > 5)
			selected_girl->m_AccLevel = 5;
		else
			selected_girl->m_AccLevel++;

		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(accomdown_id))
	{
		if(selected_girl->m_AccLevel-1 < 0)
			selected_girl->m_AccLevel = 0;
		else
			selected_girl->m_AccLevel--;

		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(takegold_id))
	{
		take_gold(selected_girl);
	}
	if(g_InterfaceEvents.CheckButton(reldungeon_id))
	{
		g_Brothels.GetDungeon()->GetDungeonPos(selected_girl);
		if((g_Brothels.GetBrothel(g_CurrBrothel)->m_NumRooms - g_Brothels.GetBrothel(g_CurrBrothel)->m_NumGirls) == 0)
		{
			g_MessageQue.AddToQue("The current brothel has no more room.\nBuy a new one, get rid of some girls, or change the brothel you are currently managing.", 0);
		}
		else
		{
			Girl* nextGirl = remove_selected_girl();
			Girl* tempGirl = g_Brothels.GetDungeon()->RemoveGirl(g_Brothels.GetDungeon()->GetGirl(g_Brothels.GetDungeon()->GetGirlPos(selected_girl)));
			g_Brothels.AddGirl(g_CurrBrothel, tempGirl);

			if(g_Brothels.GetDungeon()->GetNumGirls() == 0)
			{
				selected_girl = nullptr;
				g_WinManager.Pop();
			}
			else
				selected_girl = nextGirl;
		}
		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(senddungeon_id))
	{
	    std::string message;
		g_Brothels.GetGirlPos(g_CurrBrothel, selected_girl);

		// does she decide to fight back
		if(g_Brothels.FightsBack(selected_girl))
		{
			bool win = true;
			sGang* gang = g_Gangs.GetGangOnMission(MISS_GUARDING);
			int count = 8;
			while(gang && win && count >= 0)
			{
				if(g_Gangs.GangCombat(selected_girl, gang))
					win = true;
				else
					win = false;
				if(gang->m_Num == 0)
					gang = g_Gangs.GetGangOnMission(MISS_GUARDING);
				count--;
				if(count<0)
					win = true;
			}
			// Calculate combat between goons and girl if she decides to fight back
			if(win)
			{
				message += "She puts up a fight";

				if(gang)
				{
					if(gang->m_Num == 0)
						message += ", and the gang is completely wiped out";
				}
				message += ". ";

				if(g_Brothels.PlayerCombat(selected_girl))				// fight with the player
				{
					// If girl wins she escapes and leaves the brothel
					message += "After defeating you as well, she escapes to the outside.\n";
					message += " She will escape for good in 6 weeks if you don't send someone after her.";

					Girl* nextGirl = remove_selected_girl();
					Girl* temp = selected_girl;
					if(selected_girl->m_DayJob != JOB_INDUNGEON)
						g_Brothels.RemoveGirl(g_CurrBrothel, selected_girl, false);
					else
						temp = g_Brothels.GetDungeon()->RemoveGirl(selected_girl);

					temp->m_RunAway = 6;	// player has 6 weeks to retreive
					temp->m_NightJob = temp->m_DayJob = JOB_RUNAWAY;

					g_Brothels.AddGirlToRunaways(temp);

				    std::string smess = "";
					smess += temp->m_Realname;
					smess += " has run away";
					g_MessageQue.AddToQue(smess, 1);

					selected_girl = nextGirl;
					if(selected_girl == nullptr)
						g_WinManager.Pop();
				}
				else	// otherwise put her in the dungeon
				{
					int reason = DUNGEON_GIRLWHIM;
					if(selected_girl->m_Spotted)
						reason = DUNGEON_GIRLSTEAL;
					Girl* nextGirl = remove_selected_girl();
					selected_girl->m_DayJob = selected_girl->m_NightJob = JOB_INDUNGEON;
					g_Brothels.RemoveGirl(g_CurrBrothel, selected_girl, false);
					g_Brothels.GetDungeon()->AddGirl(selected_girl,reason);
					message += "However, you manage to defeat her yourself and place her unconscious body in the dungeon.";

					if(g_Brothels.GetNumGirls(g_CurrBrothel) == 0)
					{
						selected_girl = nullptr;
						g_WinManager.Pop();
					}
					else
						selected_girl = nextGirl;
				}
			}
			else	// otherwise put her in the dungeon
			{
				message += "She puts up a fight ";

				if(gang)
				{
					if(gang->m_Num == 0)
						message += "and the gang is wiped out, ";
				}

				message += "but your goons manage to drag her unconscious to the dungeon.";
				int reason = DUNGEON_GIRLWHIM;
				if(selected_girl->m_Spotted)
					reason = DUNGEON_GIRLSTEAL;
				Girl* nextGirl = remove_selected_girl();
				g_Brothels.RemoveGirl(g_CurrBrothel, selected_girl, false);
				g_Brothels.GetDungeon()->AddGirl(selected_girl,reason);

				if(g_Brothels.GetNumGirls(g_CurrBrothel) == 0)
					g_WinManager.Pop();
				else
					selected_girl = nextGirl;
			}
		}
		else
		{
			int reason = DUNGEON_GIRLWHIM;
			if(selected_girl->m_Spotted)
				reason = DUNGEON_GIRLSTEAL;
			Girl* nextGirl = remove_selected_girl();
			g_Brothels.RemoveGirl(g_CurrBrothel, selected_girl, false);
			g_Brothels.GetDungeon()->AddGirl(selected_girl,reason);
			message += "She goes quietly with a sullen look on her face.";

			if(g_Brothels.GetNumGirls(g_CurrBrothel) == 0)
				g_WinManager.Pop();
			else
				selected_girl = nextGirl;
		}
		g_InitWin = true;
		g_MessageQue.AddToQue(message, 0);
		return;
	}
	if(g_InterfaceEvents.CheckButton(interact_id))
	{
		if(g_TalkCount > 0)
		{
			DirPath dp;
			eventrunning = true;
			if(selected_girl->m_DayJob != JOB_INDUNGEON)
			{
				int v[2] = {1,-1};
				cTrigger* trig = nullptr;
				if(!(trig = selected_girl->m_Triggers.CheckForScript(TRIGGER_TALK, false, v)))	// trigger any girl specific talk script
				{
					// no, so trigger the default one
					dp = dp	<< "Resources"
						<< "Scripts"
						<< "DefaultInteractDetails.script"
					;
				}
				else
				{
					// yes, so use that instead
					dp = dp << "Resources"
						<< "Characters"
						<< selected_girl->m_Name
						<< trig->m_Script
					;
				}
			}
			else
			{
				int v[2] = {0,-1};
				cTrigger* trig = nullptr;
				if(!(trig = selected_girl->m_Triggers.CheckForScript(TRIGGER_TALK, false, v)))	// trigger any girl specific talk script
				{
					// no, so trigger the default one
					dp = dp	<< "Resources"
						<< "Scripts"
						<< "DefaultInteractDungeon.script"
					;
				}
				else
				{
					dp = dp << "Resources"
						<< "Characters"
						<< selected_girl->m_Name
						<< trig->m_Script
					;
				}
			}
			cScriptManager script_manager;
			script_manager.Load(dp, selected_girl);
			if(!g_Cheats)
				g_TalkCount--;
		}
		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(next_id))
	{
		NextGirl();
	}
	if(g_InterfaceEvents.CheckButton(prev_id))
	{
		PrevGirl();
	}
}


bool cScreenGirlDetails::GirlDead(Girl *dgirl)
{
	if(g_Girls.GetStat(dgirl, STAT_HEALTH) == 0)
	{
		g_MessageQue.AddToQue("This girl is dead. She isn't going to work anymore and her body will be removed by the end of the week.", 1);
		return true;
	}
	else
		return false;
}

void cScreenGirlDetails::RefreshJobList()
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
		}
//	}
}


void cScreenGirlDetails::PrevGirl()
{
	selected_girl = get_prev_girl();
	g_InitWin = true;
}

void cScreenGirlDetails::NextGirl()
{
	selected_girl = get_next_girl();
	g_InitWin = true;
}

/*
 * return previous girl in the sorted list
 */
Girl *cScreenGirlDetails::get_prev_girl()
{
	Girl *prev_girl = nullptr;

	if (cycle_girls.size() == 0)					// Myr: Found this case from an exception. Will test to see if this
		return prev_girl;							//      is a good fix.

	if(cycle_pos <= 0)
		cycle_pos = cycle_girls.size() - 1;
	else if (unsigned(cycle_pos) >= cycle_girls.size() - 1)  // Myr: Deals with an exception encountered when there were 3 dead and 1 live
		cycle_pos = cycle_girls.size() - 2;        //      girls in the dungeon.
	else
		cycle_pos--;

	if(selected_girl->m_DayJob == JOB_INDUNGEON)
		prev_girl = g_Brothels.GetDungeon()->GetGirl(cycle_girls[cycle_pos])->m_Girl;
	else
		prev_girl = g_Brothels.GetGirl(g_CurrBrothel, cycle_girls[cycle_pos]);

	return prev_girl;
}

/*
 * return next girl in the sorted list
 */
Girl *cScreenGirlDetails::get_next_girl()
{
	Girl *next_girl = nullptr;

	if (cycle_girls.size() == 0) // Myr: Found this case from an exception.
		return next_girl;

	if (cycle_pos <= 0) // Myr: Just in case
		cycle_pos = 0;
	if(cycle_pos >= (int)cycle_girls.size() - 1)
		cycle_pos = 0;
	else
		cycle_pos++;

	if(selected_girl->m_DayJob == JOB_INDUNGEON)
		next_girl = g_Brothels.GetDungeon()->GetGirl(cycle_girls[cycle_pos])->m_Girl;
	else
		next_girl = g_Brothels.GetGirl(g_CurrBrothel, cycle_girls[cycle_pos]);

	return next_girl;
}

/*
 * the selected girl is to be removed from the current list; returns next selected girl
 */
Girl *cScreenGirlDetails::remove_selected_girl()
{
	Girl *next_girl = nullptr;

	if(cycle_girls.size() == 0) {
		return nullptr;
	}

	int cur_id = cycle_girls[cycle_pos];

	cycle_girls.erase(cycle_girls.begin()+cycle_pos);  // remove her

	if(cycle_pos >= (int)cycle_girls.size())
	{  // if this girl was the last in the list, move list position -1 if possible
		if(cycle_pos > 0)
			cycle_pos--;
		else  // or, maybe we have no more girls in the list
			return next_girl;
	}

	if(selected_girl->m_DayJob == JOB_INDUNGEON)
		next_girl = g_Brothels.GetDungeon()->GetGirl(cycle_girls[cycle_pos])->m_Girl;
	else
		next_girl = g_Brothels.GetGirl(g_CurrBrothel, cycle_girls[cycle_pos]);

	for(int i = cycle_girls.size(); i --> 0; )
	{  // all girls with ID higher than removed girl need their ID reduced
		if(cycle_girls[i] > cur_id)
			cycle_girls[i]--;
	}

	return next_girl;
}


/*
 * returns TRUE if the girl won
 */
bool cScreenGirlDetails::do_take_gold(Girl *girl, std::string &message)
{
	// TODO: checking bool against integer
	const int GIRL_LOSES = false;
	const int GIRL_WINS = true;
	bool girl_win_flag = GIRL_WINS;
/*
 *	we're taking the girl's gold. Life gets complicated if she
 *	makes a fight of it - so lets do the case where she meekly complies
 *	first
 */
	if(!g_Brothels.FightsBack(girl)) {
		message += "She quietly allows you to take her gold.";
		return GIRL_LOSES;	// no fight -> girl lose
	}
/*
 *	OK - she's going to fight -
 */
	sGang* gang;
/*
 *	ok: to win she needs to defeat all the gangs on
 *	guard duty. I've made a change here so that she doesn't
 *	need to wipe one gang out before moving on to the next one
 *	which means that she can cause some damage on the way out
 *	without necessarily slaying all who stand before her
 *
 *	it also means that if you have 5 gangs guarding, she needs
 *	to get past 5 gangs, but you don't have to have them all die
 *	in the process
 */
	while((gang = g_Gangs.GetGangOnMission(MISS_GUARDING)))
	{
/*
 *		this returns true if the girl wins, false if she loses
 *
 *		Suggestion on the forums that we allow clever girls to
 *		outwit stupid gang memebers here, which sounds cool.
 *		Also nice would be if a strongly magical girl could
 *		use sorcery to evade a none-too-crafty goon squad.
 *		(possibly make her fight the first one).
 *
 *		But none of this makes much difference if the user
 *		never sees it happen. We can make combat as textured as
 *		we like, but unless the details are reported to the player
 *		we might as well roll a single die and be done with it.
 */
		girl_win_flag = g_Gangs.GangCombat(girl, gang);
/*
 *		if she didn't win, exit the loop
 */		// TODO: checking bool against integer
		if(girl_win_flag == GIRL_LOSES) {
			break;
		}
	}
/*
 *	the "girl lost" case is easier
 */ // TODO: checking bool against integer
	if(girl_win_flag == GIRL_LOSES) {		// put her in the dungeon
		message += "She puts up a fight ";
		if(gang && gang->m_Num == 0) {
			message += "and the gang is wiped out, ";
		}
		message += " but you take her gold anyway.";
		return girl_win_flag;
	}
/*
 *	from here on down, the girl won against the goons
 */
	message += "She puts up a fight and ";
	if(gang && gang->m_Num == 0) {
		message += "the gang is wiped out";
	}
/*
 *	can the player tame this particular shrew?
 */
	if(!g_Brothels.PlayerCombat(girl))	// fight with the player
	{
		message += "But you defeat her yourself and take her gold.";
		return false;	// girl did not win, after all
	}
/*
 *	Looks like she won: put her out of the brothel
 *	and post her as a runaway
 */
	message += "after defeating you as well she escapes to the outside.\n";

	Girl* nextGirl = remove_selected_girl();
	Girl* temp = girl;
/*
 *	what we have to do depends on whether she was in brothel
 *	or dungeon
 */
	if(girl->m_DayJob != JOB_INDUNGEON)
		g_Brothels.RemoveGirl(g_CurrBrothel, girl, false);
	else
		temp = g_Brothels.GetDungeon()->RemoveGirl(girl);
/*
 *	set her job
 */
	temp->m_RunAway = 6;	// player has 6 weeks to retreive
	temp->m_NightJob = girl->m_DayJob = JOB_RUNAWAY;
/*
 *	add her to the runaway list
 */
	g_Brothels.AddGirlToRunaways(temp);

    std::string smess = "";
	smess += temp->m_Realname;
	smess += " has run away";
	g_MessageQue.AddToQue(smess, 1);

	selected_girl = nextGirl;
	g_InitWin = true;

	if(selected_girl == nullptr)
		g_WinManager.Pop();

	return true;	// the girl still won
}

void cScreenGirlDetails::take_gold(Girl *girl)
{
    std::string message;
	bool girl_win = do_take_gold(girl, message);
/*
 *	if the girl won, then we're pretty much sorted
 *	display the message and return
 */
	if(girl_win) {
		g_MessageQue.AddToQue(message, 0);
		g_InitWin = true;
		return;
	}
/*
 *	so the girl lost: take away her money now
 */
	g_Gold.misc_credit(girl->m_Money);
	girl->m_Money = 0;
/*
 *	she isn't going to like this much
 *	but it's not so bad for slaves
 */
	if(girl->m_States&(1<<STATUS_SLAVE))
	{
		g_Girls.UpdateStat(girl, STAT_CONFIDENCE, -1);
		g_Girls.UpdateStat(girl, STAT_OBEDIENCE, 5);
		g_Girls.UpdateStat(girl, STAT_SPIRIT, -2);
		g_Girls.UpdateStat(girl, STAT_PCHATE, 5);
		g_Girls.UpdateStat(girl, STAT_PCLOVE, -5);
		g_Girls.UpdateStat(girl, STAT_PCFEAR, 5);
		g_Girls.UpdateStat(girl, STAT_HAPPINESS, -20);
	}
	else
	{
		g_Girls.UpdateStat(girl, STAT_CONFIDENCE, -5);
		g_Girls.UpdateStat(girl, STAT_OBEDIENCE, 5);
		g_Girls.UpdateStat(girl, STAT_SPIRIT, -10);
		g_Girls.UpdateStat(girl, STAT_PCHATE, 30);
		g_Girls.UpdateStat(girl, STAT_PCLOVE, -30);
		g_Girls.UpdateStat(girl, STAT_PCFEAR, 10);
		g_Girls.UpdateStat(girl, STAT_HAPPINESS, -50);
	}
/*
 *	and queue the message again
 */
	g_MessageQue.AddToQue(message, 0);
	g_InitWin = true;
	return;
}

} // namespace WhoreMasterRenewal
