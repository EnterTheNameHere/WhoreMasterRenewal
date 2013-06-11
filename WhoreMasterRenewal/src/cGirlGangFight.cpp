
#include "cGirlGangFight.h"
#include "cBrothel.h"
#include "cGangs.h"
#include "cGirls.h"
#include "cRng.h"
#include "CLog.h"

cGirlGangFight::cGirlGangFight(Girl *girl)
{
	m_girl	= girl;
	m_girl_stats	= 0;
/*
 *	set this up on the basis that she refuses to fight
 */
	m_goon_stats	= 0;
	m_max_goons 	= 0;
//	m_ratio	 		= 0.0;
//	m_dead_goons 	= 0;
	m_girl_fights	= false;
	m_girl_wins		= false;
	m_wipeout		= false;
	m_unopposed		= false;
	m_player_wins	= false;
/*
 *	decide if she's going to fight or flee
 */
	if(!g_Brothels.FightsBack(m_girl)) {
		return;
	}
	m_girl_fights	= true;
/*
 *	ok, she fights. Find all the gangs on guard duty
 */
	m_girl_wins = false;
	std::vector<sGang*> v = g_Gangs.gangs_on_mission(MISS_GUARDING);
/*
 *	no gang, so girl wins. PC combat is outside this class ATM
 */
	if(v.size() == 0) {
		m_girl_wins = true;
		m_unopposed = true;
		return;
	}
/*
 *	we'll take goons from a random gang - distributes the casualties a bit
 *	more evenly for multi-select brandings and the like
 */
	int index = g_Dice.in_range(0, v.size()-1);
	g_LogFile.ss() << "\ncGirlGangFight: random gang index = " << index;
	g_LogFile.ssend();
	sGang *gang = v[index];
	g_LogFile.ss() << "\ncGirlGangFight: gang = " << gang->m_Name;
	g_LogFile.ssend();
/*
 *	4 + 1 for each gang on guard duty
 *	that way there's a benefit to multiple gangs guarding
 */
 	m_max_goons = 4 + v.size();
/*
 *	to the maximum of the number in the gang
 */
	if(m_max_goons > gang->m_Num) {
		m_max_goons = gang->m_Num;
	}
/*
 *	now - sum the girl and gang stats
 *	we're not going to average the gangs.
 *	yes this gives them an unfair advantage
 *	that's the point of having 5:1 odds :)
 */
	m_girl_stats = m_girl->combat() + m_girl->magic() + m_girl->intelligence();
/*
 *	Now the gangs. I'm not factoring the girl's health
 *	because there's something dramatically satisfying
 *	about her breeaking out of the dungeon after being 
 *	tortured near unto death, and then still beating the
 *	thugs up. You'd buy into it in a Hollywood blockbuster...
 *
 *	Annnnyway....
 */
	m_goon_stats = *g_Gangs.GetWeaponLevel() * 5 * m_max_goons;
	for(int i = 0; i < m_max_goons; i++) {
		m_goon_stats +=	gang->combat() +
				gang->magic() +
				gang->intelligence()
		;
	}
/*
 *	the girl's base chance of winning is determined by the stat ratio
 */
	m_odds = 1.0 * m_girl_stats / (m_goon_stats + m_girl_stats);
/*
 *	let's add some trait based bonuses
 *	I'm not going to do any that are already reflected in stat values
 *	(so no "Psychic" bonus, no "Tough" either)
 *	we can streamline this with the trait overhaul
 */
 	if(m_girl->has_trait("Clumsy"))		m_odds -= 0.05;
 	if(m_girl->has_trait("Broken Will"))	m_odds -= 0.10;
 	if(m_girl->has_trait("Meek"))		m_odds -= 0.05;
 	if(m_girl->has_trait("Dependant"))	m_odds -= 0.10;
 	if(m_girl->has_trait("Fearless"))	m_odds += 0.10;
 	if(m_girl->has_trait("Fleet of Foot"))	m_odds += 0.10;
/*
 *	get it back into the 0 <= N <= 1 range
 */
 	if(m_odds < 0) m_odds = 0;
 	if(m_odds > 1) m_odds = 1;
/*
 *	roll the dice! If it passes then the girl wins
 */
	int pc = static_cast <int> (m_odds * 100.0);
	int roll = g_Dice.in_range(0, 100);
 	g_LogFile.ss() << "GirlGangFight: %    = " << pc << "\n";
 	g_LogFile.ss() << "GirlGangFight: roll = " << roll;
	g_LogFile.ssend();
 	if(roll >= pc) {
		lose_vs_own_gang(gang);
		return;
	}
	if(!g_Brothels.PlayerCombat(girl)) {
		m_girl_wins = false;
		m_player_wins = true;
		return;
	}
	win_vs_own_gang(gang);
}

void cGirlGangFight::lose_vs_own_gang(sGang* gang)
{
 	g_LogFile.ss() << "GirlGangFight: girl loses";
	g_LogFile.ssend();
	m_girl_wins = false;
/*
 *	She's going to get hurt some. Moderating this, we have the fact that
 *	the goons don't really want to damage her (lest the boss get all shouty)
 *	However, depending on how good she is, they may not have a choice
 *
 *	also, I don't want to kill her if she was a full health. I hate it when
 *	that happens. You can send a girl to the dungeons and she's dead before
 *	you've even had a chance to twirl your moustaches at her.
 *	So we need to know how much health she had in the first place
 */
	int start_health = m_girl->health();
	int damage = (10 + g_Dice.random(12)) * m_max_goons;
/*
 *	and if the gangs had the advantage, reduce the
 *	damage actually taken accordingly
 */
 	if(m_goon_stats > m_girl_stats) {
		damage *= m_girl_stats;
		damage /= m_goon_stats;
	}
/*
 *	lastly, make sure this isn't going to kill her, if her health was
 *	> 90 in the first place
 */
 	if(start_health >= 90 && damage >= start_health) {
		damage = start_health - 1;
	}
	m_girl->health(-damage);
/*
 *	need a bit more detail here, really...
 */
	int casualties = use_potions(gang, g_Dice.in_range(1,6) - 3);
	gang->m_Num -= casualties;
	g_LogFile.ss() << "adjusted gang casualties = " << casualties;
	g_LogFile.ssend();
}

int cGirlGangFight::use_potions(sGang* /*gang*/, int casualties)
{
/*
 *	if there's zero casualties - nothing do to
 *	and we won't reduce below 1 for potions
 */
	if(casualties < 2) {
		if(casualties < 0) casualties = 0;
		return casualties;
	}
/*
 *	maximum deaths prevented by potions is one less than the total
 *	or the total number of potions - whichever is less
 */
	int max = casualties - 1;
	int *pots_pt = g_Gangs.GetHealingPotions();
	if(*pots_pt < max) {
		max = *pots_pt;
	}
/*
 *	reduction random number in that range 
 */
	int reduction = g_Dice.in_range(1,max);
	*pots_pt -= reduction;
	casualties -= reduction;
	if(casualties < 0) casualties = 0;
	return casualties;
}

void cGirlGangFight::win_vs_own_gang(sGang* gang)
{
 	g_LogFile.ss() << "GirlGangFight: girl wins";
	g_LogFile.ssend();
	m_girl_wins = true;
/*
 *	Give her some damage from the combat. She won, so don't kill her.
 *	should really modify this for traits. And agility.
 */
	int damage = g_Dice.random(10) * m_max_goons;
	m_girl->health(-damage);
	if(m_girl->health() == 0) m_girl->health(1);
/*
 *	now - how many goons die today?
 *	I'm assuming the girl's priority is to escape
 *	rather than "clearing the level" so let's have a base of
 *	1-5
 */
 	int casualties = g_Dice.in_range(1,6);
/*
 *	some more trait mods. Assasin adds an extra dice since
 *	she's been trained to deliver the coup-de-grace on a helpless
 *	opponent.
 */
	if(m_girl->has_trait("Assassin")) {
		casualties += g_Dice.in_range(1,6);
	}
	if(m_girl->has_trait("Adventurer")) {	// some level clearing instincts
		casualties += 2;
	}
	if(m_girl->has_trait("Merciless")) casualties ++;
	if(m_girl->has_trait("Yandere")) casualties ++;
	if(m_girl->has_trait("Tsundere")) casualties ++;
	if(m_girl->has_trait("Meek")) casualties --;
	if(m_girl->has_trait("Dependent")) casualties -= 2;
/*
 *	fleet of foot means she gets out by running away more than fighting
 *	so fewer fatalities
 */
	if(m_girl->has_trait("Fleet of Foot")) casualties -= 2;
/*
 *	OK, apply the casualties and make sure it doesn't go negative
 *	allow for the effect of potions, first
 */
	casualties = use_potions(gang, casualties);
 	gang->m_Num -= casualties;
	if(gang->m_Num < 0) {
		gang->m_Num = 0;
		
	}
}


