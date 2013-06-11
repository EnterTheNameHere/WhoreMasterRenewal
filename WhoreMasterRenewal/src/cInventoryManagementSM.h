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
#ifndef CINVENTORYMANAGEMENTSM_H_INCLUDED_1525
#define CINVENTORYMANAGEMENTSM_H_INCLUDED_1525
#pragma once

class Girl;

class cInventoryManagementSM
{
public:
	void init();
	void handle_events();
	void item_shift_r();
	Girl* get_girl_from_list(int owner_idx, int list_id);

private:
	int leftOwner;
	int rightOwner;
	int NumBrothelGirls;
	int NumDungeonGirls;
	int filter;
	int leftItem;
	int rightItem;
};

#endif // CINVENTORYMANAGEMENTSM_H_INCLUDED_1525
