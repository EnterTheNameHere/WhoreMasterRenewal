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
#ifndef CGIRL_H_INCLUDED_1528
#define CGIRL_H_INCLUDED_1528
#pragma once

#include "Constants.h"
#include "cEvents.h" // required cEvents
#include "cTriggers.h" // required cTriggerList
#include "cNameList.h" // required cNameList

#include <map>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

class cGirls;
extern cGirls g_Girls;
class cAbstractGirls;
extern cAbstractGirls* g_GirlsPtr;
class sGirl;
extern sGirl* MarketSlaveGirls[8];
extern int MarketSlaveGirlsDel[8];
extern sGirl* selected_girl;
extern std::vector<int> cycle_girls;
extern int cycle_pos;

class cIndexedList;
class TiXmlElement;
class cPlayer;
class cAnimatedSurface;
class CSurface;
struct sCustomer;
struct sGang;
struct sInventoryItem;
struct sBrothel;
struct sTrait;

typedef uint8_t	Uint8; // Used by SDL

class cAbstractGirls {
public:
    virtual ~cAbstractGirls() {};
    
	virtual int GetStat(sGirl* girl, int stat)=0;
	virtual int GetSkill(sGirl* girl, int skill)=0;
	virtual void UpdateStat(sGirl* girl, int stat, int amount)=0;
	virtual void UpdateSkill(sGirl* girl, int skill, int amount)=0;
	virtual bool CalcPregnancy(
		sGirl* girl, int chance, int type,
		unsigned char stats[NUM_STATS],
		unsigned char skills[NUM_SKILLS]
	)=0;
//	virtual void AddTrait(sGirl* girl, std::string name, bool temp = false, bool removeitem = false, bool remember = false)=0;
	virtual bool AddTrait(sGirl* girl, std::string name, bool temp = false, bool removeitem = false, bool remember = false)=0;
	virtual bool HasTrait(sGirl* girl, std::string name)=0;
	virtual void UpdateTempSkill(sGirl* girl, int skill, int amount)=0;	// updates a skill temporarily
	virtual void UpdateTempStat(sGirl* girl, int stat, int amount)=0;
};



// structure to hold randomly generated girl information
typedef struct sRandomGirl
{
    std::string m_Name;
    std::string m_Desc;

	unsigned char m_Human;	                    // 0 means they are human otherwise they are not
	unsigned char m_Catacomb;	                // 1 means they are a monster found in catacombs, 0 means wanderer

	unsigned char m_MinStats[NUM_STATS];	    // min and max stats they may start with
	unsigned char m_MaxStats[NUM_STATS];

	unsigned char m_MinSkills[NUM_SKILLS];	    // min and max skills they may start with
	unsigned char m_MaxSkills[NUM_SKILLS];

	unsigned char m_NumTraits;	                // number of traits they are assigned
	sTrait* m_Traits[MAXNUM_TRAITS];	        // List of traits they may start with
	unsigned char m_TraitChance[MAXNUM_TRAITS];	// the percentage change for each trait

	int m_MinMoney;	// min and max money they can start with
	int m_MaxMoney;

	sRandomGirl* m_Next;
/*
 *	MOD: DocClox Sun Nov 15 06:11:43 GMT 2009
 *	stream operator for debugging
 *	plus a shitload of XML loader funcs
 */
	friend std::ostream& operator<<(std::ostream& os, sRandomGirl& g);
/*
 *	one func to load the girl node,
 *	and then one each for each embedded node
 *
 *	Not so much difficult as tedious.
 */
	void load_from_xml(TiXmlElement*);
	void process_trait_xml(TiXmlElement*);
	void process_stat_xml(TiXmlElement*);
	void process_skill_xml(TiXmlElement*);
	void process_cash_xml(TiXmlElement*);
/*
 *	END MOD
 */
	static sGirl* lookup;  // used to look up stat and skill IDs
	sRandomGirl();
	~sRandomGirl();
}sRandomGirl;



// Character image
class cImage
{
public:
	cImage();
	~cImage();

	cImage* m_Next;
	std::shared_ptr<CSurface> m_Surface;
	cAnimatedSurface* m_AniSurface;
};

// Character image management class
class cImageList
{
public:
	cImageList();
	~cImageList();

	void Free();

	bool AddImage(std::string filename, std::string path = "", std::string file = "");
	int DrawImage(int x, int y, int width, int height, bool random, int img);
	std::shared_ptr<CSurface> GetImageSurface(bool random, int& img);
	cAnimatedSurface* GetAnimatedSurface(int& img);
	bool IsAnimatedSurface(int& img);
	std::string GetName(int i);

	int m_NumImages;
	cImage* m_Images;
	cImage* m_LastImages;
};

class cAImgList	// class that manages a set of images from a directory
{
public:
	cAImgList();
	~cAImgList();

	std::string m_Name;	// name of the directory containing the images
	cImageList m_Images[NUM_IMGTYPES];	// the images

	cAImgList* m_Next;
};


class cImgageListManager	// manages all the girl images
{
public:
	cImgageListManager();
	~cImgageListManager();

	void Free();

	cAImgList* ListExists(std::string name);	// returns the list if the list is already loaded, returns 0 if it is not
	cAImgList* LoadList(std::string name);	// loads a list if it doensn't already exist and returns a pointer to it. returns pointer to list if it does exist

private:
	cAImgList* m_First;
	cAImgList* m_Last;
};

typedef struct sChild
{
	enum Gender {
		None	= -1,
		Girl	=  0,
		Boy	=  1
	};
	Gender m_Sex;

	std::string boy_girl_str() {
		if(m_Sex == Boy)
			return "boy";

		return "girl";
	}
	bool is_boy()	{ return m_Sex == Boy; }
	bool is_girl()	{ return m_Sex == Girl; }

	unsigned char m_Age;	// grows up at 60 weeks
	unsigned char m_IsPlayers;	// 1 when players
	unsigned char m_Unborn;	// 1 when child is unborn (for when stats are inherited from customers)

	// skills and stats from the father
	unsigned char m_Stats[NUM_STATS];
	unsigned char m_Skills[NUM_SKILLS];

	sChild* m_Next;
	sChild* m_Prev;

	sChild(bool is_players = false, Gender gender = None);
	~sChild();

	TiXmlElement* SaveChildXML(TiXmlElement* pRoot);
	bool LoadChildXML(TiXmlHandle hChild);

}sChild;
/*
a class to handle all the child related code to prevent errors.
*/
class cChildList
{
public:
    cChildList();
	~cChildList();
    
	sChild* m_FirstChild;
	sChild* m_LastChild;
	int m_NumChildren;
	void add_child(sChild*);
	sChild* remove_child(sChild*,sGirl*);
	//void handle_childs();
	//void save_data(std::ofstream);
	//void write_data(std::ofstream);
	//sChild * GenerateBornChild();//need to figure out what the player/customer base class is and if needed create one
	//sChild * GenerateUnbornChild();

};


// Represents a single girl
struct sGirl
{
	char* m_Name;								// The girls name
	std::string m_Realname;							// this is the name displayed in text
/*
 *	MOD: changed from char* -- easier to change from lua -- doc
 */
	std::string m_Desc;								// Short story about the girl

	unsigned char m_NumTraits;					// current number of traits they have
	sTrait* m_Traits[MAXNUM_TRAITS];			// List of traits they have
	unsigned char m_TempTrait[MAXNUM_TRAITS];	// a temp trait if not 0. Trait removed when == 0. traits last for 20 weeks.

	unsigned char m_NumRememTraits;				// number of traits that are apart of the girls starting traits
	sTrait* m_RememTraits[MAXNUM_TRAITS*2];		// List of traits they have inbuilt

	unsigned char m_DayJob;						// id for what job the girl is currently doing
	unsigned char m_NightJob;					// id for what job the girl is currently doing
	unsigned char m_PrevDayJob;					// id for what job the girl was doing
	unsigned char m_PrevNightJob;				// id for what job the girl was doing

	//ADB needs to be int because player might have more than 256
	int m_NumInventory;							// current amount of inventory they have
	sInventoryItem* m_Inventory[40];			// List of inventory items they have (40 max)
	unsigned char m_EquipedItems[40];			// value of > 0 means equipped (wearing) the item

	long m_States;								// Holds the states the girl has

	// Abstract stats (not shown as numbers but as a raiting)
	unsigned char m_Stats[NUM_STATS];
	int m_StatMods[NUM_STATS];					// perminant modifiers to stats
	int m_TempStats[NUM_STATS];					// these go down (or up) by 30% each week until they reach 0

	int m_Enjoyment[NUM_ACTIONTYPES];			// these values determine how much a girl likes an action
												// (-100 is hate, +100 is loves)
	bool m_Virgin;								// is she a virgin

	bool m_UseAntiPreg;							// if true she will use anit preg measures

	unsigned char m_Withdrawals;				// if she is addicted to something this counts how many weeks she has been off

	int m_Money;

	unsigned char m_AccLevel;					// how good her accomadation is, 0 is slave like and non-slaves will really hate it

	unsigned char m_Skills[NUM_SKILLS];
	int m_SkillMods[NUM_SKILLS];
	int m_TempSkills[NUM_SKILLS];				// these go down (or up) by 1 each week until they reach 0

	unsigned char m_RunAway;					// if 0 then off, if 1 then girl is removed from list,
												// otherwise will count down each week
	unsigned char m_Spotted;					// if 1 then she has been seen stealing but not punished yet

	unsigned long m_WeeksPast;					// number of weeks in your service
	unsigned int m_BDay;						// number of weeks in your service since last aging

	unsigned long m_NumCusts;					// number of customers this girl has slept with

	bool m_Tort;								// if true then have already tortured today
	bool m_JustGaveBirth;						// did she give birth this current week?

	int m_Pay;									// used to keep track of pay this turn

	cAImgList* m_GirlImages;					// Images to display

	long m_FetishTypes;							// the types of fetishes this girl has

	unsigned char m_Flags[NUM_GIRLFLAGS];		// flags used by scripts

	cEvents m_Events;							// Each girl keeps track of all her events that happened to her in the last turn


	cTriggerList m_Triggers;					// triggers for the girl

	unsigned char m_DaysUnhappy;				// used to track how many days they are really unhappy for

	sGirl* m_Next;
	sGirl* m_Prev;

	unsigned char m_WeeksPreg;					// number of weeks pregnant or inseminated
	unsigned char m_PregCooldown;				// number of weeks until can get pregnant again
	cChildList m_Children;

	sGirl();
	~sGirl();

	void dump(std::ostream &os);

/*
 *	MOD: docclox. attach the skill and stat names to the
 *	class that uses them. Plus an XML load method and
 *	an ostream << operator to pretty print the struct for
 *	debug purposes.
 *
 *	Sun Nov 15 05:58:55 GMT 2009
 */
	static const char* stat_names[];
	static const char* skill_names[];
	static const char* status_names[];
/*
 *	again, might as well make them part of the struct that uses them
 */
	static const unsigned int max_stats;
	static const unsigned int max_skills;
	static const unsigned int max_statuses;
/*
 *	we need to be able to go the other way, too:
 *	from string to number. The maps map stat/skill names
 *	onto index numbers. The setup flag is so we can initialise
 * 	the maps the first time an sGirl is constructed
 */
	static bool m_maps_setup;
	static std::map<std::string, unsigned int> stat_lookup;
	static std::map<std::string, unsigned int> skill_lookup;
	static std::map<std::string, unsigned int> status_lookup;
	static void setup_maps();

	static int lookup_stat_code(std::string s);
	static int lookup_skill_code(std::string s);
	static int lookup_status_code(std::string s);
/*
 *	Strictly speaking, methods don't belong in structs.
 *	I've always thought that more of a guideline than a hard and fast rule
 */
	void load_from_xml(TiXmlElement *el);
	TiXmlElement* SaveGirlXML(TiXmlElement* pRoot);
	bool LoadGirlXML(TiXmlHandle hGirl);

/*
 *	stream operator - used for debug
 */
	friend std::ostream& operator<<(std::ostream& os, sGirl& g);
/*
 *	it's a bit daft that we have to go through the global g_Girls
 *	every time we want a stat.
 *
 *	I mean the sGirl type is the one we're primarily concerned with.
 *	that ought to be the base for the query.
 *
 *	Of course, I could just index into the stat array,
 *	but I'm not sure what else the cGirls method does.
 *	So this is safer, if a bit inefficient.
 */
 	bool calc_pregnancy(int,cPlayer*);
 	int get_stat(int stat_id);
 	int upd_temp_stat(int stat_id, int amount);
 	int upd_stat(int stat_id, int amount);
/*
 *	Now then:
 */
	int charisma()		{ return get_stat(STAT_CHARISMA); }
	int charisma(int n)	{ return upd_stat(STAT_CHARISMA, n); }
	int happiness()		{ return get_stat(STAT_HAPPINESS); }
	int happiness(int n)	{ return upd_stat(STAT_HAPPINESS, n); }
	int libido()		{ return get_stat(STAT_LIBIDO); }
	int libido(int n)	{ return upd_stat(STAT_LIBIDO, n); }
	int constitution()	{ return get_stat(STAT_CONSTITUTION); }
	int constitution(int n)	{ return upd_stat(STAT_CONSTITUTION, n); }
	int intelligence()	{ return get_stat(STAT_INTELLIGENCE); }
	int intelligence(int n)	{ return upd_stat(STAT_INTELLIGENCE, n); }
	int confidence()	{ return get_stat(STAT_CONFIDENCE); }
	int confidence(int n)	{ return upd_stat(STAT_CONFIDENCE, n); }
	int mana()		{ return get_stat(STAT_MANA); }
	int mana(int n)		{ return upd_stat(STAT_MANA, n); }
	int agility()		{ return get_stat(STAT_AGILITY); }
	int agility(int n)	{ return upd_stat(STAT_AGILITY, n); }
	int fame()		{ return get_stat(STAT_FAME); }
	int fame(int n)		{ return upd_stat(STAT_FAME, n); }
	int level()		{ return get_stat(STAT_LEVEL); }
	int level(int n)	{ return upd_stat(STAT_LEVEL, n); }
	int askprice()		{ return get_stat(STAT_ASKPRICE); }
	int askprice(int n)	{ return upd_stat(STAT_ASKPRICE, n); }
	/* It's NOT lupus! */
	int house()		{ return get_stat(STAT_HOUSE); }
	int house(int n)	{ return upd_stat(STAT_HOUSE, n); }
	int exp()		{ return get_stat(STAT_EXP); }
	int exp(int n)		{ return upd_stat(STAT_EXP, n); }
	int age()		{ return get_stat(STAT_AGE); }
	int age(int n)		{ return upd_stat(STAT_AGE, n); }
	int obedience()		{ return get_stat(STAT_OBEDIENCE); }
	int obedience(int n)	{ return upd_stat(STAT_OBEDIENCE, n); }
	int spirit()		{ return get_stat(STAT_SPIRIT); }
	int spirit(int n)	{ return upd_stat(STAT_SPIRIT, n); }
	int beauty()		{ return get_stat(STAT_BEAUTY); }
	int beauty(int n)	{ return upd_stat(STAT_BEAUTY, n); }
	int tiredness()		{ return get_stat(STAT_TIREDNESS); }
	int tiredness(int n)	{ return upd_stat(STAT_TIREDNESS, n); }
	int health()		{ return get_stat(STAT_HEALTH); }
	int health(int n)	{ return upd_stat(STAT_HEALTH, n); }
	int pcfear()		{ return get_stat(STAT_PCFEAR); }
	int pcfear(int n)	{ return upd_stat(STAT_PCFEAR, n); }
	int pclove()		{ return get_stat(STAT_PCLOVE); }
	int pclove(int n)	{ return upd_stat(STAT_PCLOVE, n); }
	int pchate()		{ return get_stat(STAT_PCHATE); }
	int pchate(int n)	{ return upd_stat(STAT_PCHATE, n); }
/*
 *	notice that if we do tweak get_stat to reference the stats array
 *	direct, the above still work.
 *
 *	similarly...
 */
 	int get_skill(int skill_id);
 	int upd_temp_skill(int skill_id, int amount);
 	int upd_skill(int skill_id, int amount);
	int	anal()		{ return get_skill(SKILL_ANAL); }
	int	anal(int n)	{ return upd_skill(SKILL_ANAL, n); }
	int	bdsm()		{ return get_skill(SKILL_BDSM); }
	int	bdsm(int n)	{ return upd_skill(SKILL_BDSM, n); }
	int	beastiality()	{ return get_skill(SKILL_BEASTIALITY); }
	int	beastiality(int n){ return upd_skill(SKILL_BEASTIALITY, n); }
	int	combat()	{ return get_skill(SKILL_COMBAT); }
	int	combat(int n)	{ return upd_skill(SKILL_COMBAT, n); }
	int	group()		{ return get_skill(SKILL_GROUP); }
	int	group(int n)	{ return upd_skill(SKILL_GROUP, n); }
	int	lesbian()	{ return get_skill(SKILL_LESBIAN); }
	int	lesbian(int n)	{ return upd_skill(SKILL_LESBIAN, n); }
	int	magic()		{ return get_skill(SKILL_MAGIC); }
	int	magic(int n)	{ return upd_skill(SKILL_MAGIC, n); }
	int	normalsex()	{ return get_skill(SKILL_NORMALSEX); }
	int	normalsex(int n){ return upd_skill(SKILL_NORMALSEX, n); }
	int	service()	{ return get_skill(SKILL_SERVICE); }
	int	service(int n)	{ return upd_skill(SKILL_SERVICE, n); }
	int	strip()		{ return get_skill(SKILL_STRIP); }
	int	strip(int n)	{ return upd_skill(SKILL_STRIP, n); }


/*
 *	convenience func. Also easier to read like this
 */
	bool carrying_monster();
	bool carrying_human();
	bool carrying_players_child();
	bool carrying_customer_child();
	bool is_pregnant();
	bool is_mother();
	bool is_poisoned();
	void clear_pregnancy();
	int preg_chance(int base_pc, bool good=false, double factor=1.0);
	bool calc_pregnancy(cPlayer *player, bool good=false, double factor=1.0);
	bool calc_pregnancy(sCustomer *cust, bool good=false, double factor=1.0);
	bool calc_insemination(sCustomer *cust, bool good=false, double factor=1.0);
	bool calc_insemination(cPlayer *player, bool good=false, double factor=1.0);
/*
 *	let's overload that...
 *	should be able to do the same using sCustomer as well...
 */
	void add_trait(std::string trait, bool temp = true);
	bool has_trait(std::string trait);
	bool is_addict();
	sChild* next_child(sChild* child, bool remove=false);
	int preg_type(int image_type);
	sGirl* run_away();

	bool is_slave();
	bool is_free();
	void set_slave();
	bool is_monster();
	bool is_human();

	void fight_own_gang(bool& girl_wins);
	void win_vs_own_gang(std::vector<sGang*> &v, int max_goons, bool& girl_wins);
	void lose_vs_own_gang(
		std::vector<sGang*> &v,
		int max_goons,
		int girl_stats,
		int gang_stats,
		bool &girl_wins
	);

	void OutputGirlRow(std::string* Data, const std::vector<std::string>& columnNames);
	void OutputGirlDetailString(std::string& Data, const std::string& detailName);

	// END MOD
};

class GirlPredicate {
public:
    virtual ~GirlPredicate() {}
	virtual bool test(sGirl*) { return true; }
};

// Keeps track of all the available (not used by player) girls in the game.
class cGirls : public cAbstractGirls
{
public:
	cGirls();
	virtual ~cGirls();

	void Free();

	void LoadDefaultImages();
/*
 *	load the templated girls
 *	(if loading a save game doesn't load from the global template,
 *	loads from the save games' template)
 *
 *	LoadGirlsDecider is a wrapper function that decides to load
 *	XML or Legacy formats.
 *	LoadGirlsXML loads the XML files
 *	LoadGirlsLegacy is the original load function. More or less.
 */
	void LoadGirlsDecider(std::string filename);
	void LoadGirlsXML(std::string filename);
	void LoadGirlsLegacy(std::string filename);
/*
 *	SaveGirls doesn't seem to be the inverse of LoadGirls
 *	but rather writes girl data to the save file
 */
	TiXmlElement* SaveGirlsXML(TiXmlElement* pRoot);	// Saves the girls to a file
	bool LoadGirlsXML(TiXmlHandle hGirls);
	void LoadGirlsLegacy(std::ifstream& ifs);
	void LoadGirlLegacy(sGirl* current, std::ifstream& ifs);

	void AddGirl(sGirl* girl);		// adds a girl to the list
	void RemoveGirl(sGirl* girl, bool deleteGirl = false);	// Removes a girl from the list (only used with editor where all girls are available)

	sGirl* GetGirl(int girl);	// gets the girl by count

	void GirlFucks(sGirl* girl, int DayNight, sCustomer* customer, bool group, std::string& message, u_int& SexType);	// does the logic for fucking
	// MYR: Millions of ways to say, [girl] does [act] to [customer]
	std::string GetRandomGroupString();
	std::string GetRandomSexString();
	std::string GetRandomLesString();
	std::string GetRandomBDSMString();
	std::string GetRandomBeastString();
	std::string GetRandomAnalString();

	// MYR: More functions for attack/defense/agility-style combat.
	int GetCombatDamage(sGirl *girl, int CombatType);
	int TakeCombatDamage(sGirl* girl, int amt);

	void LevelUp(sGirl* girl);	// advances a girls level
	void LevelUpStats(sGirl* girl); // Functionalized stat increase for LevelUp
	void UpdateStat(sGirl* girl, int stat, int amount);	// updates a stat
	void LoadGirlImages(sGirl* girl);	// loads a girls images using her name to check that directory in the characters folder
	void ApplyTraits(sGirl* girl, sTrait* trait = nullptr, bool rememberflag = false);	// applys the stat bonuses for traits to a girl
	void UnapplyTraits(sGirl* girl, sTrait* trait = nullptr);	// unapplys a trait (or all traits) from a girl
	bool PossiblyGainNewTrait(sGirl* girl, std::string Trait, int Threshold, int ActionType, std::string Message, bool DayNight);
	//int UnapplyTraits(sGirl* girl, sTrait* trait = nullptr);	// unapplys a trait (or all traits) from a girl
	void UpdateSkill(sGirl* girl, int skill, int amount);	// updates a skill
	void UpdateEnjoyment(sGirl* girl, int whatSheEnjoys, int amount, bool wrapTo100 = false); //updates what she enjoys
	int DrawGirl(sGirl* girl, int x, int y, int width, int height, int ImgType, bool random = true, int img = 0);	// draws a image of a girl
	std::shared_ptr<CSurface> GetImageSurface(sGirl* girl, int ImgType, bool random, int& img, bool gallery=false);	// draws a image of a girl
	cAnimatedSurface* GetAnimatedSurface(sGirl* girl, int ImgType, int& img);
	bool IsAnimatedSurface(sGirl* girl, int ImgType, int& img);
	bool HasTrait(sGirl* girl, std::string trait);
	bool HasRememberedTrait(sGirl* girl, std::string trait);
	int GetNumSlaveGirls();
	int GetNumCatacombGirls();
	int GetSlaveGirl(int from);
	int GetStat(sGirl* girl, int stat);
	int GetSkill(sGirl* girl, int skill);
	void SetSkill(sGirl* girl, int skill, int amount);
	void SetStat(sGirl* girl, int stat, int amount);
	void UpdateTempSkill(sGirl* girl, int skill, int amount);	// updates a skill temporarily
	void UpdateTempStat(sGirl* girl, int stat, int amount);	// updates a stat temporarily
	int GetRebelValue(sGirl* girl, bool matron);
	void EquipCombat(sGirl* girl);  // girl makes sure best armor and weapons are equipped, ready for combat
	void UnequipCombat(sGirl* girl);  // girl unequips armor and weapons, ready for brothel work or other non-aggressive jobs
	bool RemoveInvByNumber(sGirl* girl, int Pos);

	Uint8 girl_fights_girl(sGirl* a, sGirl* b);

	bool InheritTrait(sTrait* trait);

	void AddRandomGirl(sRandomGirl* girl);
/*
 *	mod - docclox
 *	same deal here: LoadRandomGirl is a wrapper
 *	The "-Legacy" version is the original
 *	The "-XML" version is the new one that loads from XML files
 */
	void LoadRandomGirl(std::string filename);
	void LoadRandomGirlXML(std::string filename);
	void LoadRandomGirlLegacy(std::string filename);
	// end mod

	sGirl* CreateRandomGirl(int age, bool addToGGirls, bool slave = false, bool undead = false, bool NonHuman = false, bool childnaped = false);

	sGirl* GetRandomGirl(bool slave = false, bool catacomb = false);

	bool NameExists(std::string name);

	bool CheckInvSpace(sGirl* girl) {if(girl->m_NumInventory == 40)return false;return true;}
	int AddInv(sGirl* girl, sInventoryItem* item);
	bool EquipItem(sGirl* girl, int num, bool force);
	bool CanEquip(sGirl* girl, int num, bool force);
	int GetWorseItem(sGirl* girl, int type, int cost);
	int GetNumItemType(sGirl* girl, int Type);
	void SellInvItem(sGirl* girl, int num);
	void UseItems(sGirl* girl);
	int HasItem(sGirl* girl, std::string name);
//	void RemoveTrait(sGirl* girl, std::string name, bool addrememberlist = false, bool force = false);
	bool RemoveTrait(sGirl* girl, std::string name, bool addrememberlist = false, bool force = false);
	void RemoveRememberedTrait(sGirl* girl, std::string name);
	void RemoveAllRememberedTraits(sGirl* girl);					// WD: Cleanup remembered traits on new girl creation
	int GetNumItemEquiped(sGirl* girl, int Type);
	bool IsItemEquipable(sGirl* girl, int num);
	bool IsInvFull(sGirl* girl);

	int GetSkillWorth(sGirl* girl);

	bool DisobeyCheck(sGirl* girl, int action, sBrothel* brothel = nullptr);

	std::string GetDetailsString(sGirl* girl, bool purchace = false);
	std::string GetMoreDetailsString(sGirl* girl);
	std::string GetGirlMood(sGirl* girl);

//	void AddTrait(sGirl* girl, std::string name, bool temp = false, bool removeitem = false, bool inrememberlist = false);
	bool AddTrait(sGirl* girl, std::string name, bool temp = false, bool removeitem = false, bool inrememberlist = false);
	void AddRememberedTrait(sGirl* girl, std::string name);

	cImgageListManager* GetImgManager() {return &m_ImgListManager;}

	void CalculateAskPrice(sGirl* girl, bool vari);

	void AddTiredness(sGirl* girl);

	void SetAntiPreg(sGirl* girl, bool useAntiPreg) {girl->m_UseAntiPreg = useAntiPreg;}

	bool GirlInjured(sGirl* girl, unsigned int modifier);

	void CalculateGirlType(sGirl* girl);	// updates a girls fetish type based on her traits and stats
	bool CheckGirlType(sGirl* girl, int type);	// Checks if a girl has this fetish type

	void do_abnormality(sGirl *sprog, int chance);
	void HandleChild(sGirl* girl, sChild* child, std::string& summary);
	void HandleChild_CheckIncest(sGirl* mum, sGirl *sprog, sChild* child, std::string& summary);
	bool child_is_grown(sGirl* girl, sChild* child, std::string& summary, bool PlayerControlled = true);
	bool child_is_due(sGirl* girl, sChild* child, std::string& summary, bool PlayerControlled = true);
	void HandleChildren(sGirl* girl, std::string& summary, bool PlayerControlled = true);	// ages children and handles pregnancy
	bool CalcPregnancy(sGirl* girl, int chance, int type, unsigned char stats[NUM_STATS], unsigned char skills[NUM_SKILLS]);	// checks if a girl gets pregnant
	void UncontrolledPregnancies();	// ages children and handles pregnancy for all girls not controlled by player

	// mod - docclox - func to return random girl N in the chain
	// returns null if n out of range
	sRandomGirl* random_girl_at(u_int n);
/*
 *	while I'm on, a few funcs to factor out some common code in DrawImages
 */
	int num_images(sGirl* girl, int image_type);
	int get_modified_image_type(sGirl* girl, int image_type, int preg_type);
	int draw_with_default(
		sGirl* girl,
		int x, int y,
		int width, int height,
		int ImgType,
		bool random,
		int img
	);
	int calc_abnormal_pc(sGirl* mom, sGirl* sprog, bool is_players);

	std::vector<sGirl*>  get_girls(GirlPredicate* pred);

	// end mod

	// WD:	Consolidate common code in BrothelUpdate and DungeonUpdate to fn's
	void updateGirlAge(sGirl* girl, bool inc_inService = false);
	void updateTempStats(sGirl* girl);
	void updateTempSkills(sGirl* girl);
	void updateTempTraits(sGirl* girl);
	void updateSTD(sGirl* girl);
	void updateHappyTraits(sGirl* girl);
	void updateGirlTurnStats(sGirl* girl);



private:
	unsigned int m_NumGirls;	// number of girls in the class
	sGirl* m_Parent;	// first in the list of girls who are dead, gone or in use
	sGirl* m_Last;	// last in the list of girls who are dead, gone or in use

	unsigned int m_NumRandomGirls;
	sRandomGirl* m_RandomGirls;
	sRandomGirl* m_LastRandomGirls;

	// These are the default images used when a character is missing images for that particular purpose
	cAImgList* m_DefImages;
	cImgageListManager m_ImgListManager;
	cNameList names;
};

#endif  // CGIRL_H_INCLUDED_1528
