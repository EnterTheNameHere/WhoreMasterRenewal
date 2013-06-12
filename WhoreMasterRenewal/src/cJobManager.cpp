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

#include "cJobManager.h"
#include "cBrothel.h"
#include "cCustomers.h"
#include "cRng.h"
#include "cInventory.h"
#include "sConfig.h"
#include "cRival.h"
#include "CLog.h"
#include "cTrainable.h"
#include "cTariff.h"
#include "cGold.h"
#include "cGangs.h"
#include "cMessageBox.h"
#include "cGirls.h"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"

#include <algorithm>
#include <sstream>

namespace WhoreMasterRenewal
{

cJobManager::~cJobManager()
{
    ;
}

void cJobManager::Setup()
{
	// much simplier then trying to get the sJob working with this (plus a smaller memory footprint...?maybe)
	// - General
	JobFunctions[JOB_RESTING] = &WorkFreetime;
	JobFunctions[JOB_TRAINING] = &WorkTraining;
	JobFunctions[JOB_CLEANING] = &WorkCleaning;
	JobFunctions[JOB_SECURITY] = &WorkSecurity;
	JobFunctions[JOB_ADVERTISING] = &WorkAdvertising;
	JobFunctions[JOB_MATRON] = &WorkMatron;
	JobFunctions[JOB_TORTURER] = &WorkTorturer;
	JobFunctions[JOB_EXPLORECATACOMBS] = &WorkExploreCatacombs;
	JobFunctions[JOB_BEASTCAPTURE] = &WorkBeastCapture;
	JobFunctions[JOB_BEASTCARER] = &WorkBeastCare;
	// - Brothel
	JobFunctions[JOB_WHOREBROTHEL] = &WorkWhore;
	JobFunctions[JOB_WHORESTREETS] = &WorkWhore;
	JobFunctions[JOB_BROTHELSTRIPPER] = &WorkBrothelStripper;
	JobFunctions[JOB_MASSEUSE] = &WorkBrothelMasseuse;	// ************** TODO
	// - Gambling Hall
	JobFunctions[JOB_CUSTOMERSERVICE] = &WorkHall;
	JobFunctions[JOB_WHOREGAMBHALL] = &WorkWhore;
	JobFunctions[JOB_DEALER] = &WorkHall;
	JobFunctions[JOB_ENTERTAINMENT] = &WorkHall;
	JobFunctions[JOB_XXXENTERTAINMENT] = &WorkHall;
	// - Bar
	JobFunctions[JOB_BARMAID] = &WorkBar;
	JobFunctions[JOB_WAITRESS] = &WorkBar;
	JobFunctions[JOB_STRIPPER] = &WorkBar;
	JobFunctions[JOB_WHOREBAR] = &WorkWhore;
	JobFunctions[JOB_SINGER] = &WorkBar;
	// - Community Centre
	JobFunctions[JOB_COLLECTDONATIONS] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_FEEDPOOR] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_MAKEITEMS] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_SELLITEMS] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_COMUNITYSERVICE] = &WorkVoid;	// ************** TODO
	// - Drug Lab (these jobs gain bonus if in same building as a clinic)
	JobFunctions[JOB_VIRASPLANTFUCKER] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_SHROUDGROWER] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_FAIRYDUSTER] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_DRUGDEALER] = &WorkVoid;	// ************** TODO
	// - Alchemist lab (these jobs gain bonus if in same building as a drug lab)
	JobFunctions[JOB_FINDREGENTS] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_BREWPOTIONS] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_POTIONTESTER] = &WorkVoid;	// ************** TODO
	// - Arena (these jobs gain bonus if in same building as a clinic)
	JobFunctions[JOB_FIGHTBEASTS] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_WRESTLE] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_FIGHTTODEATH] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_FIGHTVOLUNTEERS] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_COLLECTBETS] = &WorkVoid;	// ************** TODO
	// - Skills Centre
	JobFunctions[JOB_TEACHBDSM] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_TEACHSEX] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_TEACHBEAST] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_TEACHMAGIC] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_TEACHCOMBAT] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_DAYCARE] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_SCHOOLING] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_TEACHDANCING] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_TEACHSERVICE] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_TRAIN] = &WorkVoid;	// ************** TODO
	// - Clinic
	JobFunctions[JOB_DOCTOR] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_GETABORT] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_PHYSICALSURGERY] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_HEALING] = &WorkVoid;	// ************** TODO
	JobFunctions[JOB_REPAIRSHOP] = &WorkVoid;	// ************** TODO


/*
 *	Names and descriptions of each job and job filter, along with first job index for each job filter
 */
	JobFilterName[JOBFILTER_GENERAL] = "General";
	JobFilterDescription[JOBFILTER_GENERAL] = "These are general jobs that work individually in any building.";
	JobFilterIndex[JOBFILTER_GENERAL] = JOB_RESTING;
	JobName[JOB_RESTING] = "Free Time";
	JobDescription[JOB_RESTING] = "She will take some time off, maybe do some shopping or walk around town. If the girl is unhappy she may try to escape.";
	JobName[JOB_TRAINING] = "Practice Skills";
	JobDescription[JOB_TRAINING] = "She will train either alone or with others to improve her skills.";
	JobName[JOB_CLEANING] = "Cleaning";
	JobDescription[JOB_CLEANING] = "She will clean the building, as filth will put off some customers.";
	JobName[JOB_SECURITY] = "Security";
	JobDescription[JOB_SECURITY] = "She will patrol the building, stopping mis-deeds.";
	JobName[JOB_ADVERTISING] = "Advertising";
	JobDescription[JOB_ADVERTISING] = "She will advertise the building's features in the city.";
	JobName[JOB_MATRON] = "Matron";
	JobDescription[JOB_MATRON] = "This girl will look after the other girls. Only non-slave girls can have this position and you must pay them 300 gold per week. Also, it takes up both shifts. (max 1)";
	JobName[JOB_TORTURER] = "Torturer";
	JobDescription[JOB_TORTURER] = "She will torture the prisoners in addition to your tortures, she will also look after them to ensure they don't die. (max 1 for all brothels)";
	JobName[JOB_EXPLORECATACOMBS] = "Explore Catacombs";
	JobDescription[JOB_EXPLORECATACOMBS] = "She will explore the catacombs looking for treasure and capturing monsters and monster girls. Needless to say, this is a dangerous job.";
	JobName[JOB_BEASTCAPTURE] = "Beast Capture";
	JobDescription[JOB_BEASTCAPTURE] = "She will go out into the wilds to capture beasts. (max 2)";
	JobName[JOB_BEASTCARER] = "Beast Carer";
	JobDescription[JOB_BEASTCARER] = "She will look after the needs of the beasts in your possession. (max 1)";

	JobFilterName[JOBFILTER_BROTHEL] = "Brothel";
	JobFilterDescription[JOBFILTER_BROTHEL] = "These are jobs for running a brothel in the current building.";
	JobFilterIndex[JOBFILTER_BROTHEL] = JOB_WHOREBROTHEL;
	JobName[JOB_WHOREBROTHEL] = "Whore in Brothel";
	JobDescription[JOB_WHOREBROTHEL] = "She will whore herself to customers within the building's walls. This is safer but a little less profitable.";
	JobName[JOB_WHORESTREETS] = "Whore on Streets";
	JobDescription[JOB_WHORESTREETS] = "She will whore herself on the streets. It is more dangerous than whoring inside but more profitable.";
	JobName[JOB_BROTHELSTRIPPER] = "Stripper in Brothel";
	JobDescription[JOB_BROTHELSTRIPPER] = "She will strip for the customers.";
	JobName[JOB_MASSEUSE] = "Masseuse in Brothel";
	JobDescription[JOB_MASSEUSE] = "She will give massages to the customers.";

	JobFilterName[JOBFILTER_GAMBHALL] = "Gambling Hall";
	JobFilterDescription[JOBFILTER_GAMBHALL] = "These are jobs for running a gambling hall in the current building.";
	JobFilterIndex[JOBFILTER_GAMBHALL] = JOB_CUSTOMERSERVICE;
	JobName[JOB_CUSTOMERSERVICE] = "Customer Service";
	JobDescription[JOB_CUSTOMERSERVICE] = "She will look after customer needs.";
	JobName[JOB_WHOREGAMBHALL] = "Whore for Gamblers";
	JobDescription[JOB_WHOREGAMBHALL] = "She will give her sexual favors to the customers.";
	JobName[JOB_DEALER] = "Game Dealer";
	JobDescription[JOB_DEALER] = "She will manage a game in the gambling hall. (requires 3)";
	JobName[JOB_ENTERTAINMENT] = "Entertainer";
	JobDescription[JOB_ENTERTAINMENT] = "She will provide entertainment to the customers.";
	JobName[JOB_XXXENTERTAINMENT] = "XXX Entertainer";
	JobDescription[JOB_XXXENTERTAINMENT] = "She will provide sexual entertainment to the customers.";

	JobFilterName[JOBFILTER_BAR] = "Bar";
	JobFilterDescription[JOBFILTER_BAR] = "These are jobs for running a bar.";
	JobFilterIndex[JOBFILTER_BAR] = JOB_BARMAID;
	JobName[JOB_BARMAID] = "Barmaid";
	JobDescription[JOB_BARMAID] = "She will staff the bar and serve drinks.";
	JobName[JOB_WAITRESS] = "Waitress";
	JobDescription[JOB_WAITRESS] = "She will bring drinks and food to customers at the tables.";
	JobName[JOB_STRIPPER] = "Stripper in Bar";
	JobDescription[JOB_STRIPPER] = "She will strip on the tables and stage for the customers.";
	JobName[JOB_WHOREBAR] = "Whore in Bar";
	JobDescription[JOB_WHOREBAR] = "She will provide sex to the customers.";
	JobName[JOB_SINGER] = "Singer";
	JobDescription[JOB_SINGER] = "She will sing for the customers.";

	JobFilterName[JOBFILTER_COMMUNITYCENTRE] = "Community Centre";
	JobFilterDescription[JOBFILTER_COMMUNITYCENTRE] = "These are jobs for running a community centre.";
	JobFilterIndex[JOBFILTER_COMMUNITYCENTRE] = JOB_COLLECTDONATIONS;
	JobName[JOB_COLLECTDONATIONS] = "Collect Donations";
	JobDescription[JOB_COLLECTDONATIONS] = "She will collect money to help the poor.";
	JobName[JOB_FEEDPOOR] = "Feed Poor";
	JobDescription[JOB_FEEDPOOR] = "She will work in a soup kitchen.";
	JobName[JOB_MAKEITEMS] = "Make Crafts";
	JobDescription[JOB_MAKEITEMS] = "She will craft cheap handmade items.";
	JobName[JOB_SELLITEMS] = "Sell Crafts";
	JobDescription[JOB_SELLITEMS] = "She will go out and sell previously crafted items.";
	JobName[JOB_COMUNITYSERVICE] = "Community Service";
	JobDescription[JOB_COMUNITYSERVICE] = "She will go around town and help out where she can.";

	JobFilterName[JOBFILTER_DRUGLAB] = "Drug Lab";
	JobFilterDescription[JOBFILTER_DRUGLAB] = "These are jobs for running a movie studio.";
	JobFilterIndex[JOBFILTER_DRUGLAB] = JOB_VIRASPLANTFUCKER;
	JobName[JOB_VIRASPLANTFUCKER] = "Fuck Viras Plants";
	JobDescription[JOB_VIRASPLANTFUCKER] = "She will allow Viras plants to inseminate her, creating Vira Blood.";
	JobName[JOB_SHROUDGROWER] = "Grow Shroud Mushrooms";
	JobDescription[JOB_SHROUDGROWER] = "She will grow Shroud Mushrooms, which require a lot of care and may explode.";
	JobName[JOB_FAIRYDUSTER] = "Fairy Duster";
	JobDescription[JOB_FAIRYDUSTER] = "She will capture fairies and grind them into a fine powder known as Fairy Dust.";
	JobName[JOB_DRUGDEALER] = "Drug Dealer";
	JobDescription[JOB_DRUGDEALER] = "She will go out and sell your drugs on the streets.";

	JobFilterName[JOBFILTER_ALCHEMISTLAB] = "Alchemist Lab";
	JobFilterDescription[JOBFILTER_ALCHEMISTLAB] = "These are jobs for running an alchemist lab.";
	JobFilterIndex[JOBFILTER_ALCHEMISTLAB] = JOB_FINDREGENTS;
	JobName[JOB_FINDREGENTS] = "Find Reagents";
	JobDescription[JOB_FINDREGENTS] = "She will go out and look for promising potion ingredients.";
	JobName[JOB_BREWPOTIONS] = "Brew Potions";
	JobDescription[JOB_BREWPOTIONS] = "She will brew a random potion using available ingredients.";
	JobName[JOB_POTIONTESTER] = "Guinea Pig";
	JobDescription[JOB_POTIONTESTER] = "She will test out random potions. Who knows what might happen? (requires 1)";

	JobFilterName[JOBFILTER_ARENA] = "Arena";
	JobFilterDescription[JOBFILTER_ARENA] = "These are jobs for running an arena.";
	JobFilterIndex[JOBFILTER_ARENA] = JOB_FIGHTBEASTS;
	JobName[JOB_FIGHTBEASTS] = "Fight Beasts";
	JobDescription[JOB_FIGHTBEASTS] = "She will fight to the death against beasts you own. Dangerous.";
	JobName[JOB_WRESTLE] = "Wrestle";
	JobDescription[JOB_WRESTLE] = "She will take part in a wrestling match. Not dangerous.";
	JobName[JOB_FIGHTTODEATH] = "Cage Match";
	JobDescription[JOB_FIGHTTODEATH] = "She will fight to the death against other girls. Dangerous.";
	JobName[JOB_FIGHTVOLUNTEERS] = "Fight Volunteers";
	JobDescription[JOB_FIGHTVOLUNTEERS] = "She will fight against customer volunteers for gold prizes. Not dangerous.";
	JobName[JOB_COLLECTBETS] = "Collect Bets";
	JobDescription[JOB_COLLECTBETS] = "She will collect bets from the customers. (requires 1)";

	JobFilterName[JOBFILTER_TRAININGCENTRE] = "Training Centre";
	JobFilterDescription[JOBFILTER_TRAININGCENTRE] = "These are jobs for running a training centre.";
	JobFilterIndex[JOBFILTER_TRAININGCENTRE] = JOB_TEACHBDSM;
	JobName[JOB_TEACHBDSM] = "Teach BDSM";
	JobDescription[JOB_TEACHBDSM] = "She will teach other girls in BDSM skills.";
	JobName[JOB_TEACHSEX] = "Teach Sex";
	JobDescription[JOB_TEACHSEX] = "She will teach other girls the ins and outs of regular sex.";
	JobName[JOB_TEACHBEAST] = "Teach Bestiality";
	JobDescription[JOB_TEACHBEAST] = "She will teach other girls how to safely fuck beasts.";
	JobName[JOB_TEACHMAGIC] = "Teach Magic";
	JobDescription[JOB_TEACHMAGIC] = "She will teach other girls how to wield magic.";
	JobName[JOB_TEACHCOMBAT] = "Teach Combat";
	JobDescription[JOB_TEACHCOMBAT] = "She will teach other girls how to fight.";
	JobName[JOB_DAYCARE] = "Daycare";
	JobDescription[JOB_DAYCARE] = "She will look after local children. (non-sexual)";
	JobName[JOB_SCHOOLING] = "Schooling";
	JobDescription[JOB_SCHOOLING] = "She will teach the local children. (non-sexual)";
	JobName[JOB_TEACHDANCING] = "Teach Dancing";
	JobDescription[JOB_TEACHDANCING] = "She will teach other girls dancing and social skills.";
	JobName[JOB_TEACHSERVICE] = "Teach Service";
	JobDescription[JOB_TEACHSERVICE] = "She will teach other girls a variety of service skills.";
	JobName[JOB_TRAIN] = "Student";
	JobDescription[JOB_TRAIN] = "She will learn from all of the other girls who are teaching here.";

	JobFilterName[JOBFILTER_CLINIC] = "Medical Clinic";
	JobFilterDescription[JOBFILTER_CLINIC] = "These are jobs for running a medical clinic.";
	JobFilterIndex[JOBFILTER_CLINIC] = JOB_DOCTOR;
	JobName[JOB_DOCTOR] = "Doctor";
	JobDescription[JOB_DOCTOR] = "She will become a doctor. Doctors earn extra cash from treating locals. (requires 1)";
	JobName[JOB_GETABORT] = "Get Abortion";
	JobDescription[JOB_GETABORT] = "She will get an abortion, removing pregnancy and/or insemination.(takes 2 days)";
	JobName[JOB_PHYSICALSURGERY] = "Cosmetic Surgery";
	JobDescription[JOB_PHYSICALSURGERY] = "She will undergo magical surgery to \"enhance\" her appearance. (takes 5 days)";
	JobName[JOB_HEALING] = "Healing";
	JobDescription[JOB_HEALING] = "She will have her wounds attended. This takes 1 day for each wound trait.";
	JobName[JOB_REPAIRSHOP] = "Repair Shop";
	JobDescription[JOB_REPAIRSHOP] = "Construct girls will be quickly repaired here.";

	// extra JobFilterIndex, needed by FOR loops (looping from JobFilterIndex[XYZ] to JobFilterIndex[XYZ+1])
	JobFilterIndex[NUMJOBTYPES] = NUM_JOBS;

}

#ifdef _DEBUG
void cJobManager::freeJobs()
{
}
#else
void cJobManager::free()
{
}
#endif

// ----- Misc

bool cJobManager::WorkVoid(Girl* girl, sBrothel* /*brothel*/, int /*DayNight*/, std::string& summary)
{
	summary += "This job isn't implemented yet";
	girl->m_Events.AddMessage("This job isn't implemented yet", IMGTYPE_PROFILE, EVENT_DEBUG);
	return false;
}

bool cJobManager::Preprocessing(int action, Girl* girl, sBrothel* brothel, int DayNight, std::string& /*summary*/, std::string& /*message*/)
{
	brothel->m_Filthiness++;
	g_Girls.AddTiredness(girl);

	if(g_Girls.DisobeyCheck(girl, action, brothel))			// they refuse to have work

	{

//		summary += "She has refused to work.\n";			// WD:	Refusal message done in cBrothelManager::UpdateGirls()
	    std::string msg = girl->m_Realname + " refused to work during the ";
		if(DayNight == 0)
			msg += "day";
		else
			msg += "night";
		msg += " Shift.";

		girl->m_Events.AddMessage(msg, IMGTYPE_PROFILE, EVENT_NOWORK);

		return true;
	}
	return false;
}

void cJobManager::GetMiscCustomer(sBrothel* brothel, sCustomer& cust)
{
	g_Customers.GetCustomer(cust, brothel);
	brothel->m_MiscCustomers+=1;
}

bool cJobManager::is_sex_type_allowed(unsigned int sex_type, sBrothel* brothel)
{
	if(sex_type == SKILL_ANAL && brothel->m_RestrictAnal)
		return false;
	else if(sex_type == SKILL_BEASTIALITY && brothel->m_RestrictBeast)
		return false;
	else if(sex_type == SKILL_BDSM && brothel->m_RestrictBDSM)
		return false;
	else if(sex_type == SKILL_NORMALSEX && brothel->m_RestrictNormal)
		return false;
	else if(sex_type == SKILL_GROUP && brothel->m_RestrictGroup)
		return false;
	else if(sex_type == SKILL_LESBIAN && brothel->m_RestrictLesbian)
		return false;

	return true;
}

// ----- Job related

std::vector<Girl*> cJobManager::girls_on_job(sBrothel *brothel, u_int job_wanted, int day_or_night)
{
	u_int job_id;
	Girl* girl;
	std::vector<Girl*> v;

	for(girl = brothel->m_Girls; girl; girl = girl->m_Next) {
		if (day_or_night == 0)
		    job_id = girl->m_DayJob;
		else
			job_id = girl->m_NightJob;
/*
 *		having sorted that out
 */
		if(job_id != job_wanted)
			continue;

		v.push_back(girl);
	}
	return v;
}

bool cJobManager::is_job_employed(sBrothel * brothel,u_int job_wanted,int day_or_night)
{
	u_int job_id;
	Girl* girl;
	for(girl = brothel->m_Girls; girl; girl = girl->m_Next) {
		if(day_or_night == 0)
			job_id = girl->m_DayJob;
		else
			job_id = girl->m_NightJob;
		if(job_id == job_wanted)
		return true;
	}
	return false;
}

void cJobManager::do_advertising(sBrothel* brothel)
{  // advertising jobs are handled before other jobs, more particularly before customer generation
	brothel->m_AdvertisingLevel = 1.0;  // base multiplier
	Girl* current = brothel->m_Girls;
	while(current)
	{
	    std::string summary = "";
		bool refused = false;
		if(current->m_DayJob == JOB_ADVERTISING)
		{
			refused = WorkAdvertising(current,brothel,0,summary);

			if(refused)	// if she refused she still gets tired
				g_Girls.AddTiredness(current);
		}
		if(current->m_NightJob == JOB_ADVERTISING)
		{
			refused = WorkAdvertising(current,brothel,1,summary);

			if(refused)	// if she refused she still gets tired
				g_Girls.AddTiredness(current);
		}
		current = current->m_Next;
	}
}

int cJobManager::get_num_on_job(int /*job*/, int /*brothel_id*/, bool /*day_or_night*/)
{
	return 0;
}

bool cJobManager::is_job_Paid_Player(u_int Job)
{
/*
 *	WD:	Tests for jobs that the player pays the wages for
 *
 *	Jobs that earn money for the player are commented out
 *
 */

	return (
		// ALL JOBS

		// - General
		Job ==	JOB_RESTING				||	// relaxes and takes some time off
		Job ==	JOB_TRAINING			||	// trains skills at a basic level
		Job ==	JOB_CLEANING			||	// cleans the building
		Job ==	JOB_SECURITY			||	// Protects the building and its occupants
		Job ==	JOB_ADVERTISING			||	// Goes onto the streets to advertise the buildings services
		Job ==	JOB_MATRON				||	// looks after the needs of the girls (only 1 allowed)
		Job ==	JOB_TORTURER			||	// Tortures the people in the dungeons to help break their will (this is in addition to player torture) (only 1 allowed)
		//Job ==	JOB_EXPLORECATACOMBS	||	// goes adventuring in the catacombs
		Job ==	JOB_BEASTCAPTURE		||	// captures beasts for use (only 2 allowed) (creates beasts resource)
		Job ==	JOB_BEASTCARER			||	// cares for captured beasts (only 1 allowed) (creates beasts resource and lowers their loss)

		// - Brothel
		//Job ==	JOB_WHOREBROTHEL		||	// whore herself inside the building
		//Job ==	JOB_WHORESTREETS		||	// whore self on the city streets
		//Job ==	JOB_BROTHELSTRIPPER		||	// strips for customers and sometimes sex
		//Job ==	JOB_MASSEUSE			||	// gives massages to patrons and sometimes sex

		// - Gambling Hall - Using WorkHall() or WorkWhore()
		//Job ==	JOB_CUSTOMERSERVICE		||	// looks after customers needs (customers are happier when people are doing this job)
		//Job ==	JOB_WHOREGAMBHALL		||	// looks after customers sexual needs
		//Job ==	JOB_DEALER				||	// dealer for gambling tables (requires at least 3 for all jobs in this filter to work)
		//Job ==	JOB_ENTERTAINMENT		||	// sings, dances and other shows for patrons
		//Job ==	JOB_XXXENTERTAINMENT	||	// naughty shows for patrons

		// - Bar Using - WorkBar() or WorkWhore()
		//Job ==	JOB_BARMAID				||	// serves at the bar (requires 1 at least)
		//Job ==	JOB_WAITRESS			||	// waits on the tables (requires 1 at least)
		//Job ==	JOB_STRIPPER			||	// strips for the customers and gives lapdances
		//Job ==	JOB_WHOREBAR			||	// offers sexual services to customers
		//Job ==	JOB_SINGER				||	// sings int the bar (girl needs singer trait)

/*
 *		Not Implemented
 */

#if 0
		// - Community Centre
		Job ==	JOB_COLLECTDONATIONS	||	// collects money to help the poor
		Job ==	JOB_FEEDPOOR			||	// work in a soup kitchen
		Job ==	JOB_MAKEITEMS			||	// makes cheap crappy handmade items for selling to raise money (creates handmade item resource)
		Job ==	JOB_SELLITEMS			||	// goes out and sells the made items (sells handmade item resource)
		Job ==	JOB_COMUNITYSERVICE		||	// Goes around town helping where they can

		// - Drug Lab (these jobs gain bonus if in same building as a clinic)
		Job ==	JOB_VIRASPLANTFUCKER	||	// the plants need to inseminate with other humanoids to proper-gate, creates vira blood items
		Job ==	JOB_SHROUDGROWER		||	// They require lots of care, and may explode. Produces shroud mushroom item.
		Job ==	JOB_FAIRYDUSTER			||	// captures and pounds faries to dust, produces fairy dust item
		Job ==	JOB_DRUGDEALER			||	// goes out onto the streets to sell the items made with the other jobs

		// - Alchemist lab (these jobs gain bonus if in same building as a drug lab)
		Job ==	JOB_FINDREGENTS			||	// goes around and finds ingredients for potions, produces ingredients resource.
		Job ==	JOB_BREWPOTIONS			||	// will randomly produce any items with the word "potion" in the name, uses ingredients resource
		Job ==	JOB_POTIONTESTER		||	// Potion tester tests potions, they may die doing this or random stuff may happen. (requires 1)

		// - Arena (these jobs gain bonus if in same building as a clinic)
		Job ==	JOB_FIGHTBEASTS			||	// customers come to place bets on who will win, girl may die (uses beasts resource)
		Job ==	JOB_WRESTLE				||	// as above no chance of dying
		Job ==	JOB_FIGHTTODEATH		||	// as above but against other girls (chance of dying)
		Job ==	JOB_FIGHTVOLUNTEERS		||	// fight against customers who volunteer for prizes of gold
		Job ==	JOB_COLLECTBETS			||	// collects the bets from customers (requires 1)

		// - Skills Centre
		Job ==	JOB_TEACHBDSM			||	// teaches BDSM skills
		Job ==	JOB_TEACHSEX			||	// teaches general sex skills
		Job ==	JOB_TEACHBEAST			||	// teaches beastiality skills
		Job ==	JOB_TEACHMAGIC			||	// teaches magic skills
		Job ==	JOB_TEACHCOMBAT			||	// teaches combat skills
		Job ==	JOB_DAYCARE				||	// looks after the local kids (non sex stuff of course)
		Job ==	JOB_SCHOOLING			||	// teaches the local kids (non sex stuff of course)
		Job ==	JOB_TEACHDANCING		||	// teaches dancing and social skills
		Job ==	JOB_TEACHSERVICE		||	// teaches misc skills
		Job ==	JOB_TRAIN				||	// trains the girl in all the disicplince for which there is a teacher

		// - Clinic
		Job ==	JOB_DOCTOR				||	// becomes a doctor (requires 1) (will make some extra cash for treating locals)
		Job ==	JOB_GETABORT			||	// gets an abortion (takes 2 days)
		Job ==	JOB_PHYSICALSURGERY		||	// magical plastic surgery (takes 5 days)
		Job ==	JOB_HEALING				||	// takes 1 days for each wound trait received.
		Job ==	JOB_REPAIRSHOP			||	// construct girls can get repaired quickly
#endif

		// - extra unassignable
		//Job ==	JOB_INDUNGEON			||
		//Job ==	JOB_RUNAWAY				||

		false );


}

std::string cJobManager::JobDescriptionCount(int job_id, int brothel_id, bool day)
{
	std::stringstream text;
	text << JobName[job_id];
	text << " (";
	text << g_Brothels.GetNumGirlsOnJob(brothel_id, job_id, day);
	text << ")";
	return text.str();
}

bool cJobManager::HandleSpecialJobs(int TargetBrothel, Girl* Girl, int JobID, int OldJobID, bool DayOrNight)
{
	bool MadeChanges = true;  // whether a special case applies to specified job or not
	// Special cases?
	if(u_int(JobID) == JOB_MATRON)
	{
		if(g_Brothels.GetNumGirlsOnJob(TargetBrothel, JOB_MATRON, DayOrNight) == 1)
			g_MessageQue.AddToQue("You can only have one matron per brothel.", 0);
		else if(Girl->m_States&(1<<STATUS_SLAVE))
			g_MessageQue.AddToQue("The matron cannot be a slave.", 0);
		else
			Girl->m_NightJob = Girl->m_DayJob = JOB_MATRON;
	}
	else if(u_int(JobID) == JOB_TORTURER)
	{
		if(g_Brothels.GetNumGirlsOnJob(-1, JOB_TORTURER, DayOrNight) == 1)
			g_MessageQue.AddToQue("You can only have one torturer among all of your brothels.", 0);
		else if(Girl->m_States&(1<<STATUS_SLAVE))
			g_MessageQue.AddToQue("The torturer cannot be a slave.", 0);
		else
			Girl->m_NightJob = Girl->m_DayJob = JOB_TORTURER;
	}
	else
	{  // Special cases were checked and don't apply, so just set the job as requested
		MadeChanges = false;
		if(DayOrNight)
			Girl->m_DayJob = JobID;
		else
			Girl->m_NightJob = JobID;
	}

	if (JobID != OldJobID)
	{  // handle instances where special job has been removed, specifically where it actually matters
		if (u_int(OldJobID) == JOB_MATRON || u_int(OldJobID) == JOB_TORTURER)
		{
			if (u_int(JobID) != JOB_MATRON && u_int(JobID) != JOB_TORTURER)
			{  // for these Day+Night jobs, switch leftover day or night job back to resting
				if (DayOrNight)
					Girl->m_NightJob = JOB_RESTING;
				else
					Girl->m_DayJob = JOB_RESTING;
			}
		}
	}

	return MadeChanges;
}

// ------ Work Related Violence fns

// MYR: Rewrote the work_related_violence to add the security guard job.
//      Girls and gangs now fight using the attack/defense/agility style combat.
//      This will have to be changed to the new style of combat eventually.
//      A new kind of weaker gang not associated with any of the enemy organizations.
//      This "gang" doesn't have weapon levels or access to healing potions.
//      They're just a bunch of toughs out to make life hard for our working girls.

/*
 * return true if violence was committed against the girl
 * false if nothing happened, or if violence was committed
 * against the customer.
 */
bool cJobManager::work_related_violence(Girl* girl, int DayNight, bool streets)
{
	cConfig cfg;
	int rape_chance = (int)cfg.prostitution.rape_brothel();
	//std::vector<sGang *> gang_v;
	std::vector<sGang *> gangs_guarding = g_Gangs.gangs_on_mission(MISS_GUARDING);

	//int gang_chance = guard_coverage(&gang_v);
	int gang_coverage = guard_coverage(&gangs_guarding);
/*
 *	the base chance of an attempted rape is higher on the
 *	streets
 */
	if(streets)
		rape_chance = (int)cfg.prostitution.rape_streets();

/*
 *	night time doubles the chance of attempted rape
 *	and reduces the chance for a gang to catch it by ten
 */
	if(DayNight == 1) {
		rape_chance *= 2;
		gang_coverage = (int)((float)gang_coverage*0.8f);
	}
/*
 *	if the player has a -ve disposition, this can
 *	scare the would-be rapist into behaving himself
 */
	if(g_Dice.percent(g_Brothels.GetPlayer()->disposition() * -1))
		rape_chance = 0;


	if(g_Dice.percent(rape_chance)) {
		/*sGang *gang = g_Gangs.random_gang(gang_v);

		if(gang_stops_rape(girl, gang, gang_chance, DayNight))
			return false;
		if(girl_fights_rape(girl, DayNight))
			return false; */

		// Get a random weak gang. These are street toughs looking to abuse a girl.
		// They have low stats and smaller numbers. Define it here so that even if
		// the security girl or defending gang is defeated, any casualties they inflicts
		// carry over to the next layer of defense.
		sGang *enemy_gang = g_Gangs.GetTempWeakGang();
		// There is also between 1 and 10 of them, not 15 every time
		enemy_gang->m_Num = g_Dice%10 + 1;

		// Three more lines of defense

		// 1. Brothel security
		if (security_stops_rape(girl, enemy_gang, DayNight))
			return false;

		// 2. Defending gangs
		if (gang_stops_rape(girl, gangs_guarding, enemy_gang, gang_coverage, DayNight))
			return false;

		// 3. The attacked girl herself
		if (girl_fights_rape(girl, enemy_gang, DayNight))
			return false;

		// If all defensive measures fail...
		customer_rape(girl);
		return true;
	}
	g_Girls.UpdateEnjoyment(girl, ACTION_SEX, +1, true);
/*
 *	the fame thing could work either way.
 *	of course, that road leads to us keeping reputation for
 *	different things - beating the customer would be a minus
 *	for the straight sex crowd, but a big pull for the
 *	femdom lovers...
 *
 *	But let's get what we have working first
 */
	if(girl->has_trait("Yandere") && g_Dice.percent(30)) {
		girl->m_Events.AddMessage("She beat the customer silly.", IMGTYPE_PROFILE, DayNight);
		g_Girls.UpdateStat(girl, STAT_FAME, -1);
	}
	return false;
}

/*
 * We need a cGuards guard manager. Or possible a cGuardsGuards manager.
 */
int cJobManager::guard_coverage(std::vector<sGang*> *vpt)
{
	int pc = 0;
	std::vector<sGang*> v = g_Gangs.gangs_on_mission(MISS_GUARDING);

	if(vpt != nullptr)
		*vpt = v;

	for(u_int i = 0; i < v.size(); i++) {
		sGang *gang = v[i];
/*
 *		our modifier is gang int - 50
 *		so that gives us a value from -50 to + 50
 */
		float mod = (float)gang->intelligence() - 50;
/*
 *		double it to make it -100 to +100
 *		and then divide by 100 to make it -1.0 to +1.0
 *
 *		can just divide by 50, really
 */
		mod /= 50;
/*
 *		add one so we have a value from 0.0 to 2.0
 */
		mod += 1;
/*
 *		and that's the multiplier
 *
 *		a base 6% per gang member give a full strength gang
 *		with average int a 90% chance of stopping a rape
 *		at 100 int they get 180.
 */
 		pc += int(6 * gang->m_Num * mod);
	}
/*
 *	with 6 brothels, that one gang gives a 15% chance to stop
 *	any given rape - rising to 30 for 100 intelligence
 *
 *	so 3 gangs on guard duty, at full strength, get you 90%
 *	coverage. And 4 get you 180 which should be bulletproof
 */
	return pc / g_Brothels.GetNumBrothels();
}

// True means security intercepted the perp(s)

bool cJobManager::security_stops_rape(Girl * girl, sGang *enemy_gang, int day_night)
{
	// MYR: Note to self: STAT_HOUSE isn't the brothel number :)
	int GirlsBrothelNo = g_Brothels.GetGirlsCurrentBrothel(girl);
	sBrothel * Brothl = g_Brothels.GetBrothel(GirlsBrothelNo);
	int SecLev = Brothl->m_SecurityLevel, OrgNumMem = enemy_gang->m_Num;
	Girl * SecGuard;

	// A gang takes 5 security points per member to stop
	if (SecLev < OrgNumMem * 5)
		return false;

	// Security guards on duty this shift
	std::vector<Girl *> SecGrd = g_Brothels.GirlsOnJob(GirlsBrothelNo, JOB_SECURITY, day_night == SHIFT_DAY);
	// Security guards with enough health to fight
	std::vector<Girl *> SecGrdWhoCanFight;

	if( SecGrd.empty() )
		return false;

	// Remove security guards who are too wounded to fight
	// Easier to work from the end
	for(int i = SecGrd.size() - 1; i >= 0 ; i--)
	{
	  SecGuard = SecGrd[i];
	  if (SecGuard->m_Stats[STAT_HEALTH] <= 50)
		  SecGrd.pop_back();
	  else
	  {
		SecGrdWhoCanFight.push_back(SecGrd.back());
		SecGrd.pop_back();
	  }
	}

	// If all the security guards are too wounded to fight
	if( SecGrdWhoCanFight.empty() )
		return false;

	// Get a random security guard
	SecGuard = SecGrdWhoCanFight.at(g_Dice%SecGrdWhoCanFight.size());


    // Most of the rest of this is a copy-paste from customer_rape
	bool res = g_Gangs.GirlVsEnemyGang(SecGuard, enemy_gang);

	// Earn xp for all kills, even if defeated
	int xp = 3;

	if (g_Girls.HasTrait(SecGuard, "Quick Learner"))
		xp += 2;
	else if (g_Girls.HasTrait(SecGuard, "Slow Learner"))
		xp -= 2;

	g_Girls.UpdateStat(SecGuard, STAT_EXP, (OrgNumMem - enemy_gang->m_Num) * xp);

	if (res)  // Security guard wins
	{
		g_Girls.UpdateSkill(SecGuard, SKILL_COMBAT, 1);
		g_Girls.UpdateSkill(SecGuard, SKILL_MAGIC, 1);
		g_Girls.UpdateStat(SecGuard, STAT_LIBIDO, OrgNumMem - enemy_gang->m_Num);  // There's nothing like killin ta make ya horny!
		g_Girls.UpdateStat(SecGuard, STAT_CONFIDENCE, OrgNumMem - enemy_gang->m_Num);
		g_Girls.UpdateStat(SecGuard, STAT_FAME, OrgNumMem - enemy_gang->m_Num);
		g_Girls.UpdateEnjoyment(girl, ACTION_COMBAT, OrgNumMem - enemy_gang->m_Num, true);
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKSECURITY, OrgNumMem - enemy_gang->m_Num, true);

		std::stringstream msg;

		// I decided to not say gang in the text. It can be confused with a player or enemy organization's
		// gang, when it is neither.
		if (OrgNumMem == 1)
		{
		  msg << "Security Report:\nA customer tried to attack " << girl->m_Realname << ", but you intercepted and beat ";
		  int roll = g_Dice%10 + 1;
		  if (roll == 5)
			  msg << "her.";
		  else
		      msg << "him.";
		}
		else
		{
		  msg << "Security Report:\n";
		  if (enemy_gang->m_Num == 0)
			msg << "A group of customers tried to attack " << girl->m_Realname << ". You intercepted and thrashed all " << OrgNumMem << " of them.";
		  else
			msg << "A group of " << OrgNumMem << " customers tried to attack " << girl->m_Realname << ". They fled after you intercepted and thrashed "
			  << (OrgNumMem - enemy_gang->m_Num) << " of them.";

		}
		SecGuard->m_Events.AddMessage(msg.str(), IMGTYPE_DEATH, EVENT_WARNING/*day_night*/);
	}
	else  // Loss
	{
		std::stringstream ss;

		ss << "Security Problem:\n" << "Trying to defend " << girl->m_Realname << ". You defeated "
		   << (OrgNumMem - enemy_gang->m_Num) << " of " << OrgNumMem << " before:\n" << SecGuard->m_Realname << GetGirlAttackedString();

		SecGuard->m_Events.AddMessage(ss.str(), IMGTYPE_DEATH, EVENT_DANGER);

		// Bad stuff
		g_Girls.UpdateStat(SecGuard, STAT_HAPPINESS, -40);
		g_Girls.UpdateStat(SecGuard, STAT_CONFIDENCE, -40);
		g_Girls.UpdateStat(SecGuard, STAT_OBEDIENCE, -10);
		g_Girls.UpdateStat(SecGuard, STAT_SPIRIT, -40);
		g_Girls.UpdateStat(SecGuard, STAT_LIBIDO, -40);
		g_Girls.UpdateStat(SecGuard, STAT_TIREDNESS, 60);
		g_Girls.UpdateStat(SecGuard, STAT_PCFEAR, 20);
		g_Girls.UpdateStat(SecGuard, STAT_PCLOVE, -20);
		g_Girls.UpdateStat(SecGuard, STAT_PCHATE, 20);
		g_Girls.GirlInjured(SecGuard, 10); // MYR: Note
		g_Girls.UpdateEnjoyment(SecGuard, ACTION_WORKSECURITY, -30, true);
		g_Girls.UpdateEnjoyment(SecGuard, ACTION_COMBAT, -30, true);
	}

	// Win or lose, subtract 5 security points per gang member
	Brothl->m_SecurityLevel = Brothl->m_SecurityLevel - OrgNumMem * 5;

	return res;
}

bool cJobManager::gang_stops_rape(Girl* girl, std::vector<sGang *> gangs_guarding, sGang *enemy_gang,
	int coverage, int day_night)
{
	if((g_Dice%100 + 1) > coverage)
		return false;

	sGang *guarding_gang = gangs_guarding.at(g_Dice%gangs_guarding.size());

	// GangVsGang returns true if enemy_gang loses
	bool guards_win = g_Gangs.GangBrawl(guarding_gang, enemy_gang);

	if(!guards_win) {
		std::stringstream gang_s, girl_s;
		gang_s << guarding_gang->m_Name << " was defeated defending " << girl->m_Realname << ".";
		girl_s << guarding_gang->m_Name << " was defeated defending you from a gang of rapists.";
		guarding_gang->m_Events.AddMessage(gang_s.str(), IMGTYPE_PROFILE,  EVENT_WARNING);
		girl->m_Events.AddMessage(girl_s.str(), IMGTYPE_DEATH, EVENT_WARNING /*day_night*/);
		return false;
	}

/*
 * options: smart guards send them off with a warning
 * dead customers should impact disposition and suspicion
 * might also need a bribe to cover it up
 */
	std::stringstream gang_ss, girl_ss;
	int roll = g_Dice.random(100) + 1;
	if(roll < guarding_gang->intelligence() / 2)
	{
		gang_ss <<	"One of the " << guarding_gang->m_Name << " found a customer getting violent with "
			<< girl->m_Realname << " and sent him packing, in fear for his life.";
		girl_ss << "Customer attempt to rape her, but guards " << guarding_gang->m_Name << " scared him off.";
	}
	else if(roll < guarding_gang->intelligence())
	{
		gang_ss	<< "One of the " << guarding_gang->m_Name << " found a customer hurting  " << girl->m_Realname
			<< " and beat him bloody before sending him away.";
		girl_ss	<< "Customer attempt to rape her, but guards " << guarding_gang->m_Name << " gave him a beating.";
	}
	else
	{
		gang_ss	<< "One of the " << guarding_gang->m_Name << " killed a customer who was trying to rape "
			<< girl->m_Realname << ".";
		girl_ss	<< "Customer attempt to rape her, but guards " << guarding_gang->m_Name << " killed them.";
	}
	g_LogFile.ss() << "gang ss=" << gang_ss.str() << std::endl
                << "girl ss=" << girl_ss.str() << std::endl;
    g_LogFile.ssend();

	girl->m_Events.AddMessage(girl_ss.str(), IMGTYPE_DEATH, day_night);
	guarding_gang->m_Events.AddMessage(gang_ss.str(), IMGTYPE_PROFILE, EVENT_GANG);

	return true;
}

#if 0
/*
 * options: smart guards send them off with a warning
 * dead customers should impact disposition and suspicion
 * might also need a bribe to cover it up
 */
bool cJobManager::gang_stops_rape(Girl* girl, sGang *gang, int chance, int day_night)
{
	if(g_Dice.percent(chance) == false)
		return false;

	bool gang_wins = false;
	int wlev = *g_Gangs.GetWeaponLevel();
/*
 *	the odds are tipped very much in favour of the gang
 *
 *	they need to blow both magic and combat rolls three times
 *	to lose - even for a new gang with about 30%, that works out
 *	about an 89% chance to win, rising to 98% with weapon level 4
 *
 *	for gangs in the 90% range, you're looking at 1 in a million chances
 */
 	int bonus = 0;
/*
 *	make the gang leader tougher
 */
	if(gang->m_Num == 1)
		bonus = 10;

	int combat = gang->combat() + 4 * wlev + bonus;
	int magic = gang->magic() + 4 * wlev + bonus;
	for(int i = 0; i < 3; i++) {
		gang_wins = g_Dice.percent(combat) || g_Dice.percent(magic) ;
		if(gang_wins)
						break;

	}
/*
 *	if the gang member lost, tell the player about it
 *	no point in subtelties of simulation if the customer
 *	never knows
 */
	if(!gang_wins) {
	    std::string s;
		if(gang->m_Num == 1)
			s =	"The leader of this gang is dead; killed attempting to prevent a rape. You may want to hire a new gang.";
		else
			s =	"One of this gang's members turned up dead. It looks like he attacked a rapist and lost.";

		gang->m_Num --;
		gang->m_Events.AddMessage(s, 0, 0);
	return false;
	}

	std::stringstream gang_ss, girl_ss;
	int roll = g_Dice.random(100) + 1;
	if(roll < gang->intelligence() / 2) {
		gang_ss <<	"One of the " <<	gang->m_Name <<	" found a customer getting violent with "
			<<	girl->m_Realname <<	" and sent him packing, in fear for his life";
		girl_ss <<	"Customer attempt to rape her " <<"but the guard scared him off";
	}
	else if(roll < gang->intelligence()) {
		gang_ss	<<	"One of the " << gang->m_Name << " found a customer hurting  " << girl->m_Realname
			<< " and beat him bloody before sending him away";
		girl_ss	<<	"Customer attempt to rape her but " << "the guard gave him a beating";
	}
	else {
		gang_ss	<<	"One of the " << gang->m_Name << " killed a customer who was trying to rape "
			<<	girl->m_Realname;
		girl_ss	<<	"Customer attempt to rape her but " << "the was killed by the guard.";
	}
	g_LogFile.ss() << "gang ss=" << gang_ss.str() << std::endl
                << "girl ss=" << girl_ss.str() << std::endl;
    g_LogFile.ssend();
	gang->m_Events.AddMessage(gang_ss.str(), 0, 0);
	girl->m_Events.AddMessage(girl_ss.str(), IMGTYPE_DEATH, day_night);
	return true;
}
#endif

// true means she won

bool cJobManager::girl_fights_rape(Girl* girl, sGang *enemy_gang, int /*day_night*/)
{
	int OrgNumMem = enemy_gang->m_Num;

	bool res = g_Gangs.GirlVsEnemyGang(girl, enemy_gang);

	// Earn xp for all kills, even if defeated
	int xp = 3;

	if (g_Girls.HasTrait(girl, "Quick Learner"))
		xp += 2;
	else if (g_Girls.HasTrait(girl, "Slow Learner"))
		xp -= 2;

	g_Girls.UpdateStat(girl, STAT_EXP, (OrgNumMem - enemy_gang->m_Num) * xp);

	if (res)  // She wins.  Yay!
	{
		g_Girls.UpdateSkill(girl, SKILL_COMBAT, 1);
		g_Girls.UpdateSkill(girl, SKILL_MAGIC, 1);
		g_Girls.UpdateStat(girl, STAT_AGILITY, 1);
		g_Girls.UpdateStat(girl, STAT_LIBIDO, OrgNumMem - enemy_gang->m_Num);  // There's nothing like killin ta make ya horny!
		g_Girls.UpdateStat(girl, STAT_CONFIDENCE, OrgNumMem - enemy_gang->m_Num);
		g_Girls.UpdateStat(girl, STAT_FAME, OrgNumMem - enemy_gang->m_Num);

		g_Girls.UpdateEnjoyment(girl, ACTION_COMBAT, OrgNumMem - enemy_gang->m_Num, true);

		std::stringstream msg;

		// MYR: I decided to not say gang in the text. It can be confused with a player or enemy organization's
		//     gang, when it is neither.
		if (OrgNumMem == 1)
		{
		  msg << "A customer tried to attack her. She killed ";
		  int roll = g_Dice%10 + 1;
		  if (roll == 5)
			  msg << "her.";
		  else
		      msg << "him.";
		}
		else
		{
		  if (enemy_gang->m_Num == 0)
		    msg << "A group of customers tried to assault her. She thrashed all " << OrgNumMem << " of them.";
		  else
			msg << "A group of " << OrgNumMem << " customers tried to assault her. They fled after she killed "
			  << (OrgNumMem - enemy_gang->m_Num) << " of them.";

		}
		girl->m_Events.AddMessage(msg.str(), IMGTYPE_DEATH, EVENT_WARNING/*day_night*/);
	}

    // Losing is dealt with later in customer_rapes (called from work_related_violence)
	return res;
}
#if 0
/*
 * Again, we can elaborate this
 *
 * some traits should give a bonus. There should be a chance of
 * dead customers. There should probably be a chance of a dead girls
 *
 * returns true if the girl is successful in fighting the rapist off
 */
bool cJobManager::girl_fights_rape(Girl* girl, int day_night)
{
    std::string msg;
	bool res = false;
	int roll = g_Dice%100;

	if(g_Girls.HasTrait(girl, "Merciless")) {
		msg =	"The customer tried to rape this girl.  She leaves him beaten and bloody, but still breathing.";
		res = true;
	}
	else if(roll < girl->combat() && roll < girl->magic()) {
		msg =	"The customer attempted to rape her but she remembered her self-defence training and fought him off";
		res = true;
	}
	girl->m_Events.AddMessage(msg, IMGTYPE_DEATH, day_night);
	return false;
}
#endif

/*
 * I think these next three could use a little detail
 * MYR: Added the requested detail (in GetGirlAttackedString() below)
 */
void cJobManager::customer_rape(Girl* girl)
{
	std::stringstream ss;

	ss << girl->m_Realname << GetGirlAttackedString();

	girl->m_Events.AddMessage(ss.str(), IMGTYPE_DEATH, EVENT_DANGER);

	// Made this more harsh, so the player hopefully notices it
	//g_Girls.UpdateStat(girl, STAT_HEALTH, -(g_Dice%10 + 5));  // Oops, can drop health below zero after combat is considered
	g_Girls.UpdateStat(girl, STAT_HAPPINESS, -40);
	g_Girls.UpdateStat(girl, STAT_CONFIDENCE, -40);
	g_Girls.UpdateStat(girl, STAT_OBEDIENCE, -10);
	g_Girls.UpdateStat(girl, STAT_SPIRIT, -40);
	g_Girls.UpdateStat(girl, STAT_LIBIDO, -40);
	g_Girls.UpdateStat(girl, STAT_TIREDNESS, 60);
	g_Girls.UpdateStat(girl, STAT_PCFEAR, 20);
	g_Girls.UpdateStat(girl, STAT_PCLOVE, -20);
	g_Girls.UpdateStat(girl, STAT_PCHATE, 20);
	g_Girls.GirlInjured(girl, 10); // MYR: Note
	g_Girls.UpdateEnjoyment(girl, ACTION_SEX, -30, true);
}
#if 0
void cJobManager::customer_rape(Girl* girl)
{
	girl->m_Events.AddMessage(girl->m_Realname + " " + GetGirlAttackedString()/*"She was beaten and raped, and the perpetrator escaped"*/, IMGTYPE_DEATH, EVENT_DANGER);
	g_Girls.UpdateStat(girl, STAT_HEALTH, -2);
	g_Girls.UpdateStat(girl, STAT_SPIRIT, -1);
	g_Girls.GirlInjured(girl, 3);
	g_Girls.UpdateEnjoyment(girl, ACTION_SEX, -8, true);
}
#endif

// MYR: Lots of different ways to say the girl had a bad day
// doc: let's have this return a string, instead of a std::stringstream:
// the caller doesn't need the stream and gcc is giving weird type coercion
// errors

std::string cJobManager::GetGirlAttackedString()
{
	int roll1 = 0, roll2 = 0, roll3 = 0;
	std::stringstream ss;

	ss << " was ";

    roll1 = g_Dice % 21 + 1;   // Remember to update this when new strings are added
	switch (roll1)
	{
	  case 1: ss << "cornered"; break;
	  case 2: ss << "hauled to the dungeon"; break;
	  case 3: ss << "put on the wood horse"; break;
	  case 4: ss << "tied to the bed"; break;
	  case 5: ss << "locked in the closet"; break;
	  case 6: ss << "drugged"; break;
	  case 7: ss << "tied up"; break;
	  case 8: ss << "wrestled to the floor"; break;
	  case 9: ss << "slapped in irons"; break;
	  case 10: ss << "cuffed"; break;
	  case 11: ss << "put in THAT machine"; break;
	  case 12: ss << "konked on the head"; break;
	  case 13: ss << "calmly sat down"; break;
	  case 14: ss << "hand-cuffed during kinky play"; break;
	  case 15: ss << "caught off guard during fellatio"; break;
	  case 16: ss << "caught cheating at cards"; break;
	  case 17: ss << "found sleeping on the job"; break;
	  case 18: ss << "chained to the porch railing"; break;
	  case 19: ss << "tied up BDSM-style"; break;
	  case 20: ss << "stretched out on the torture table"; break;
	  case 21: ss << "tied up and hung from the rafters"; break;
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
	}

	ss << " and ";

    roll2 = g_Dice % 20 + 1;
	switch (roll2)
	{
	  case 1: ss << "abused"; break;
	  case 2: ss << "whipped"; break;
	  case 3: ss << "yelled at"; break;
	  case 4: ss << "assaulted"; break;
	  case 5: ss << "raped"; break;
	  case 6: ss << "her cavities were explored"; break;
	  case 7: ss << "her hair was done"; break;
	  case 8: ss << "her feet were licked"; break;
	  case 9: ss << "relentlessly tickled"; break;
	  case 10: ss << "fisted"; break;
	  case 11: ss << "roughly fondled"; break;
	  case 12: ss << "lectured to"; break;
	  case 13: ss << "had her fleshy bits pierced"; break;
	  case 14: ss << "slapped around"; break;
	  case 15: ss << "penetrated by some object"; break;
	  case 16: ss << "shaved"; break;
	  case 17: ss << "tortured"; break;
	  case 18: ss << "forced outside"; break;
	  case 19: ss << "forced to walk on a knotted rope"; break;
	  case 20: ss << "her skin was pierced by sharp things"; break;
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
	}

	ss << " by ";

    roll3 = g_Dice % 21 + 1;
	switch (roll3)
	{
	  case 1: ss << "customers."; break;
	  case 2: ss << "some sadistic monster."; break;
	  case 3: ss << "a police officer."; break;
	  case 4: ss << "the other girls."; break;
	  case 5: ss << "Batman! (Who knew?)"; break;
	  case 6: ss << "a ghost (She claims.)"; break;
	  case 7: ss << "an enemy gang member."; break;
	  case 8: ss << "priests."; break;
	  case 9: ss << "orcs. (What?)"; break;
	  case 10: ss << "a doppleganger disguised as a human."; break;
	  case 11: ss << "a jealous wife."; break;
	  case 12: ss << "a jealous husband."; break;
	  case 13: ss << "a public health official."; break;
	  case 14: ss << "you!"; break;
	  case 15: ss << "some raving loonie."; break;
	  case 16: ss << "a ninja!"; break;
	  case 17: ss << "a pirate. (Pirates rule! Ninjas drool!)"; break;
	  case 18: ss << "members of the militia."; break;
	  case 19: ss << "your mom (It runs in the family.)"; break;
	  case 20: ss << "tentacles from the sewers."; break;
	  case 21: ss << "a vengeful family member."; break;
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
	}

	return ss.str();
}

// ------ Training

/*
 * let's look at this a little differently...
 */
void cJobManager::get_training_set(std::vector<Girl*> &v, std::vector<Girl*> &t_set)
{
	u_int max = 4;
	u_int v_siz = v.size();

	// empty out the trainable set
	t_set.clear();
/*
 *	if there's nothing in the vector, return with t_set empty
 */
 	if(v_siz == 0u)
		return;
/*
 *	if there's just one, return with t_set containing that
 */
 	if(v_siz == 1u)
	{
		t_set.push_back(v.back());
		v.pop_back();
		return;
	}
/*
 *	we want to split the girls into groups of four
 *	but if possible we don't want any singletons
 *
 *	So... if there would be one left over,
 *	we make the first group a group of three.
 *	subsequent groups will have modulus 2 and thus
 *	generate quads, until the end when we have a pair left.
 *
 *	Easier to explain in code than words, really.
 */
 	if((v_siz % 4) == 1)
		max --;		// reduce it by one, this time only
/*
 *	get the limiting variable for the loop
 *	we don't compare against v.size() because
 *	that will change as the loop progresses
 */
	u_int lim = (max < v_siz ? max : v_siz);
/*
 *	now loop until we hit max, or run out of girls
 */
 	for(u_int i = 0; i < lim; i++)
	{
		t_set.push_back(v.back());
		v.pop_back();
	}
}

bool cJobManager::WorkTraining(Girl* /*girl*/, sBrothel* /*brothel*/, int /*DayNight*/, std::string& /*summary*/)
{
	// training is already handled in UpdateGirls
	//do_training(brothel, DayNight);

	return false;
}

void cJobManager::do_solo_training(Girl *girl, int DayNight)
{
	TrainableGirl trainee(girl);
	girl->m_Events.AddMessage("She trained during this shift by herself, so learning anything worthwhile was difficult.", IMGTYPE_PROFILE, DayNight);
/*
 *	50% chance nothing happens
 */
 	if(g_Dice.percent(50))
	{
		girl->m_Events.AddMessage("Sadly, she didn't manage to improve herself in any way.", IMGTYPE_PROFILE, EVENT_SUMMARY);
		return;
	}
/*
 *	otherwise, pick a random attribute and raise it 1-3 points
 */
	int amt = 1 + g_Dice%3;
    std::string improved = trainee.update_random(amt);
	std::stringstream ss;
	ss.str("");
	ss << "She managed to gain " << amt << " " << improved << ".";
	girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_SUMMARY);
}

void cJobManager::do_training_set(std::vector<Girl*> girls, int DayNight)
{
	Girl *girl;
	std::stringstream ss;
/*
 *	we're getting a vector of 1-4 girls here
 *	(the one is possible if only one girl trains)
 */
	if(girls.empty())
	{  // no girls? that shouldn't happen
		g_LogFile.ss() << "Logic Error in cJobManager::do_training_set: empty set passed for training!";
		g_LogFile.ssend();
		return;
	}
	else if(girls.size() == 1)
	{  // special case for only one girl
		do_solo_training(girls[0], DayNight);
		return;
	}
/*
 *	OK. Now, as I was saying. We have an array of Girl* pointers..
 *	We need that to be a list of TrainableGirl objects:
 */
 	std::vector<TrainableGirl> set;
/*
 *	4 is the maximum set size. I should probably consider
 *	making that a class constant - or a static class member
 *	initialised from the config file. Later for that.
 */
	u_int num_girls = girls.size();
	for(u_int i = 0; i < num_girls; i++)
	{
		girl = girls[i];
		if(girl == nullptr)
			break;
		set.push_back(TrainableGirl(girl));
	}
/*
 *	now get an idealized composite of the girls in the set
 *
 *	and the get a vector of the indices of the most efficient
 *	three attributes for them to train
 */
	IdealGirl ideal(set);
	std::vector<int> indices = ideal.training_indices();
/*
 *	OK. Loop over the girls, and then over the indices
 */
 	for(u_int g_idx = 0; g_idx < set.size(); g_idx++)
	{
		TrainableGirl &trainee = set[g_idx];

		for(u_int i_idx = 0; i_idx < indices.size(); i_idx++)
		{
			int index = indices[i_idx];
/*
 *			the base value for any increase is the difference
 *			between the value of girl's stat, and the value of best
 *			girl in the set
 */         int aaa = ideal[index].value(); //    [index].value();
			int bbb = trainee[index].value();
			int diff = aaa - bbb;
			if(diff < 0)  // don't want a negative training value
				diff = 0;
/*
 *			plus one - just so the teacher gets a chance to
 *			learn something
 */
			diff ++;
/*
 *			divide by 10 for the increase
 */
 			int inc = (int)floor(1.0 * (double)diff / 10.0);
/*
 *			if there's any left over (or if the diff
 *			was < 10 to begin with, there's a 10%
 *			point for each difference point
 */
			int mod = diff %10;
			if(g_Dice.percent(mod * 10))
				inc ++;

			trainee[index].upd(inc);
		}

		Girl *girl = trainee.girl();
/*
 *		need to do the  "she trained hard with ..." stuff here
 */
 		//int n_girls = set.size();
		ss.str("");
		ss << "She trained during this shift in the following grouping: ";
		for(u_int i = 0; i < num_girls; i++)
		{
			ss << girls[i]->m_Realname;
			if(i == 0 && num_girls == 2)
				ss << " and ";
			else if(i < num_girls - 2)
				ss << ", ";
			else if(i == num_girls - 2)
				ss << ", and ";
		}
		ss << ".";

		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, DayNight);
		ss.str("");
		ss << "She trained during this shift";

		bool raised = false;
		for(u_int i_idx = 0; i_idx < indices.size(); i_idx++)
		{
			int index = indices[i_idx];
			int  gain = trainee[index].gain();

			if(gain == 0)
				continue;

			if(!raised)
			{
				raised = true;
				ss << " and gained ";
			}
			else
				ss << ", ";

			ss << "+" << gain <<  " " << trainee[index].name();
		}
		ss << ".";
		girl->m_Events.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_SUMMARY);
	}
}

void cJobManager::do_training(sBrothel* brothel, int DayNight)
{
	cTariff tariff;
	cConfig cfg;

	std::vector<Girl*> t_set;
	std::vector<Girl*> girls = girls_on_job(brothel, JOB_TRAINING, DayNight);

	for(u_int i = girls.size(); i --> 0; )
	{  // no girls sneaking in training if she gave birth
		if(
			(girls[i]->m_WeeksPreg > 0 && girls[i]->m_WeeksPreg+1 >= cfg.pregnancy.weeks_pregnant())
			|| (girls[i]->m_JustGaveBirth && DayNight == 1)
		)
			girls.erase(girls.begin()+i);
	}

	random_shuffle(girls.begin(),girls.end());

	for(;;)
	{
		get_training_set(girls, t_set);
		if(t_set.empty())
			break;
		do_training_set(t_set, DayNight);
	}
/*
 *	a few bookkeeping details here
 *
 *	dirt and training costs, for a start
 */
	brothel->m_Filthiness += girls.size();
	brothel->m_Finance.girl_training(
		tariff.girl_training() * girls.size()
	);
/*
 *	and then each girl gets to feel tired and horny
 *	as a result of training
 */
 	for(u_int i = 0; i < girls.size(); i++)
	{
		Girl *girl = girls[i];
		g_Girls.AddTiredness(girl);
		g_Girls.UpdateTempStat(girl, STAT_LIBIDO, 2);
	}
}

} // namespace WhoreMasterRenewal
