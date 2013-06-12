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
#ifndef CTARIFF_H_INCLUDED_1511
#define CTARIFF_H_INCLUDED_1511
#pragma once

#include "sConfig.h" // required cConfig

namespace WhoreMasterRenewal
{

struct sBrothel;
class Girl;

class cTariff
{
public:
	cTariff();

	int goon_weapon_upgrade( int );
	int goon_mission_cost( int );
	int healing_price( int );
	int nets_price( int );
	int anti_preg_price( int );
	int strip_bar_price();
	int gambling_hall_price();
	int movie_cost();
	int matron_wages( int level = 65 );
	int bar_staff_wages();
	int empty_room_cost( sBrothel* );
	int empty_bar_cost();
	int active_bar_cost( int, double );
	int empty_casino_cost( int );
	int active_casino_cost( int, double );
	int casino_staff_wages();
	int advertising_costs( int );
	int add_room_cost( int );
	double slave_price( Girl*, bool );
	int slave_buy_price( Girl* );
	int slave_sell_price( Girl* );
	int male_slave_sales();
	int creature_sales();
	int girl_training();
	int buy_facility( int );
	
private:
    cConfig config;
	double slave_base_price( Girl* );
};


} // namespace WhoreMasterRenewal

#endif // CTARIFF_H_INCLUDED_1511
