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
 
#include "cTariff.h"
#include "cGirls.h"
#include "cBrothel.h"
#include "CLog.h"
#include "cRng.h"

cTariff::cTariff()
{
    
}

int cTariff::healing_price( int n )
{
    return int( n * 10 * config.out_fact.consumables() );
}

int cTariff::nets_price( int n )
{
    return int( n * 5 * config.out_fact.consumables() );
}

int cTariff::anti_preg_price( int n )
{
    return int(n * 2 * config.out_fact.consumables() );
}

int cTariff::strip_bar_price()
{
    return int( 2500 * config.out_fact.casino_cost() );
}

int cTariff::gambling_hall_price()
{
    return int( 15000 * config.out_fact.casino_cost() );
}

int cTariff::movie_cost()
{
    return int( 100 * config.out_fact.movie_cost() );
}

/*
*	let's have matron wages go up as skill level increases.
*/
int cTariff::matron_wages( int level )
{
    int base = 200 + (level * 2);
    return int( base * config.out_fact.matron_wages() );
}

int cTariff::bar_staff_wages()
{
    return int( 20 * config.out_fact.bar_cost() );
}

int cTariff::empty_bar_cost()
{
    return int( 20 * config.out_fact.bar_cost() );
}

int cTariff::active_bar_cost( int level, double shifts )
{
    if( shifts > 2.0 ) shifts = 2.0;
    shifts /= 2.0;
    double cost = 50.0 * level / shifts;
    return int( cost * config.out_fact.bar_cost() );
}

int cTariff::empty_casino_cost( int level )
{
    return int( 50 * level * config.out_fact.casino_cost() );
}

int cTariff::active_casino_cost( int level, double shifts )
{
    if( shifts > 2.0 ) shifts = 2.0;
    shifts /= 2.0;
    double cost = 150.0 * level / shifts;
    return int( cost * config.out_fact.casino_cost() );
}

int cTariff::casino_staff_wages()
{
    g_LogFile.ss()
        << "casino wages: config factor = "
        << config.out_fact.staff_wages()
    ;
    g_LogFile.ssend();
    return int( 50 * config.out_fact.casino_cost() );
}

int cTariff::advertising_costs( int budget )
{
    return int( budget * config.out_fact.advertising() );
}

int cTariff::add_room_cost( int n )
{
    return int( n * 1000 * config.out_fact.brothel_cost() );
}

double cTariff::slave_price( Girl* girl, bool buying )
{
    if( buying )
    {
        return slave_buy_price( girl );
    }
    return slave_sell_price( girl );
}

int cTariff::male_slave_sales()
{
    return g_Dice.random(300) + 200;
}
int cTariff::creature_sales()
{
    return g_Dice.random(2000) + 100;
}

int cTariff::girl_training()
{
    return int( config.out_fact.training() * 5 );
}

/*
*	really should do this by facility and match on name
*
*	that said...
*/
int cTariff::buy_facility( int base_price )
{
    return int( config.out_fact.brothel_cost() * base_price );
}

double cTariff::slave_base_price( Girl* girl )
{
	double cost;
/*
 *	The ask price is the base price for the girl
 *	It changes with her stats, so we need to refresh it
 */
	g_Girls.CalculateAskPrice( girl, false );
/*
 *	base price is the girl's ask price stat
 */
	cost = girl->askprice() * 15;
/*
 *	add to that the sum of her skills
 */
	for( u_int i=0; i<NUM_SKILLS; i++ )
    {
		cost += (unsigned int)girl->m_Skills[i];
	}
/*
 *	virgins fetch a premium
 *	MOD: not sure of the rationale behind 158
 *	but a multiplier makes more sense to me.
 *	Let's say virgins go for half as much again
 */
	if( girl->m_Virgin )
    {
		cost *= 1.5;
	}
	
	g_LogFile.ss() << "CTariff: base price for slave '"
	  	 << girl->m_Name
		 << "' = "
		 << int(cost);
	g_LogFile.ssend();
	return cost;
}

int cTariff::slave_buy_price( Girl* girl )
{
	cConfig cfg;
	double cost = slave_base_price(girl);
	double factor = cfg.out_fact.slave_cost();

	g_LogFile.ss() << "CTariff: buy price config factor '"
	  	 << girl->m_Name
		 << "' = "
		 << factor;
	g_LogFile.ssend();
/*
 *	multiply by the config factor for buying slaves
 */
	cost *= cfg.out_fact.slave_cost();
/*
 *	a bit of debug chatter
 */
	g_LogFile.ss() << "CTariff: buy price for slave '"
	  	 << girl->m_Name
		 << "' = "
		 << int(cost);
	g_LogFile.ssend();
	return int(cost);
}

int cTariff::slave_sell_price( Girl* girl )
{
	cConfig cfg;
	double cost = slave_base_price( girl );
/*
 *	multiply by the config factor for buying slaves
 */
	return int( cost * cfg.in_fact.slave_sales() );
}

int cTariff::empty_room_cost( sBrothel* brothel )
{
	cConfig cfg;
	double cost;
/*
 *	basic cost is number of empty rooms
 *	nominal cost is 2 gold per
 *	modified by brothel support multiplier
 */
	cost = brothel->m_NumRooms - brothel->m_NumGirls;
	cost *= 2;
	cost *= cfg.out_fact.brothel_support();
	return int(cost);
}

int cTariff::goon_weapon_upgrade( int level )
{
	cConfig cfg;
	return int( (level + 1) * 1200 * cfg.out_fact.item_cost() );
}

int cTariff::goon_mission_cost( int mission )
{
	cConfig cfg;
	double cost = 0.0;
	double factor = cfg.out_fact.goon_wages();

	switch( mission )
	{
	case MISS_SABOTAGE:
	    cost = factor * 150;
	    break;
	    
	case MISS_SPYGIRLS:
	    cost = factor * 40;
	    break;
	    
	case MISS_CAPTUREGIRL:
	    cost = factor * 125;
	    break;
	    
	case MISS_EXTORTION:
	    cost = factor * 116;
	    break;
	    
	case MISS_PETYTHEFT:
	    cost = factor * 110;
	    break;
	    
	case MISS_GRANDTHEFT:
	    cost = factor * 250;
	    break;
	    
	case MISS_KIDNAPP:
	    cost = factor *	150;
	    break;
	    
	case MISS_CATACOMBS:
	    cost = factor * 300;
	    break;
	    
	case MISS_TRAINING:
	    cost = factor * 90;
	    break;
	    
	case MISS_RECRUIT:
        cost = factor * 80;
        break;
        
	default:
		g_LogFile.ss() << "Warning: cTariff: unrecogised goon mission "
			<< mission
			<< " charging as guard mission";
		g_LogFile.ssend();
        // NOTE - Fall through
            
	case MISS_GUARDING:
	    cost = factor * 60;
	    break;
	}

	if(cost > 0.0 )
    {
		return int(cost);
	}

/*
 *	if we get here, the default didn't work
 *	which shouldn't happen. I HAVE put it in a
 *	non standard place, but it should be legal.
 *
 *	Still, just to be sure...
 */
	g_LogFile.ss() << "Warning: cTariff: logic error in gang_mission_cost("
		<< mission
		<< ") - charging as guard mission";
	g_LogFile.ssend();
	return int( factor * 60 );
}
