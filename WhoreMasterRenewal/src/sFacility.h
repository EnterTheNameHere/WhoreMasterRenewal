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
#ifndef SFACILITY_H_INCLUDED_1500
#define SFACILITY_H_INCLUDED_1500
#pragma once

#include "cTariff.h" // required cTariff

namespace WhoreMasterRenewal
{

struct sFacility;

/*
 * I don't generally see the point of using chars over ints
 * for stat values - most PCs have more than enough memory to make the
 * space considerations trivial.
 *
 * that said, this would be an easy change to a template class
 * if we did want a char based version. Then a sBoundedVar would
 * use three times the memory of a char rather than 12 times, which
 * could be important if we're looking at porting to other platforms
 *
 * We could use static constants for the min and max values as well,
 * although we'd need a new subclass for each set of bounds and then to
 * override the constants. Then of course we'd probably need a virtual accessor
 * to make sure we got the right constants ... and then the vtable would
 * wipe out any savings in memory from the two ints (or chars).
 *
 * probably easier to stick with three values, all told.
 */
struct sBoundedVar
{
	int	m_min;
	int	m_max;
	int	m_curr;
/*
 *	defaults are set up for facility adjusters
 *	so range is 0-9, default to zero
 */
	sBoundedVar();
/*
 *	but we could create one with any range
 */
	sBoundedVar( int min, int max, int def = 0 );
	
	virtual ~sBoundedVar();
/*
 *	methods for adjuster buttons - simple increment with
 *	bounds checking
 */
	void up();
	void down();
/*
 *	operators = += -=
 */
	sBoundedVar& operator =( int val );
	sBoundedVar& operator +=( int val );
	sBoundedVar& operator -=( int val );
	void bound();
/*
 *	rather than a save method, just return an XML element
 *	that can be linked into a larger tree
 *
 *	The XML is going to look something like this:
 *
 *	<BoundedVar
 *		Name	= "Glitz"
 *		Min	= "0"
 *		Max	= "9"
 *		Curr	= "3"
 *	/>
 */
	TiXmlElement* to_xml( std::string name );
	bool from_xml( TiXmlElement *el );
};



/*
 * this is a bit of a mess from a model-view-controller point of view
 * but I'm not clear how to untangle it right now.
 *
 * So I'll make it work for now, and untangle it later
 */
struct sBoundedVar_Provides : public sBoundedVar
{
	int m_inc;			// increment - this per bump
	int m_space;			// space taken
	double m_slots_per_space;	// how many slots for one space?

/*
 *	we need to know how much extra space a bump would consume
 */
 	int space_needed();
/*
 *	same exercise from a slot perspective
 */
	int slots_needed();

	void init( sFacility* fac );

	TiXmlElement* to_xml( std::string name );
	bool from_xml( TiXmlElement* el );

	void up();

/*
 *	this is a little complicated
 *
 *	if a slot takes more than one space
 *	then we drop enough spaces to reduce the slot count
 *	so if the facility is a 4 space apartment suite,
 *	we drop by 4 spaces and one slot
 *
 *	on the other hand, if there are multiple slots per space
 *	we want to drop a space, and reduce the slot count to the maximum
 *	that fit in the new size. So if we get 4 kennels to the space,
 *	we reduce by 1 space and 4 kennel slots.
 */
	void down();
	int bound();
};

struct sFacility
{
    std::string m_type_name;
    std::string m_instance_name;
    std::string m_desc;
	int m_space_taken;
	int m_slots;
	int m_base_price;
	int m_paid;
	sBoundedVar_Provides m_provides;
	sBoundedVar m_glitz;
	sBoundedVar m_secure;
	sBoundedVar m_stealth;
	bool new_flag;
	cTariff tariff;

	sFacility();
	sFacility( const sFacility& f );

	void commit();

	std::string name();
    std::string desc();
    std::string type();
    
	int	space_taken();
    
	int	slots();
	int	price();
	int	glitz();
	void glitz_up();
	void glitz_down();

	int	secure();
	void secure_up();
	void secure_down();

	int	stealth();
	void stealth_up();
	void stealth_down();

	void load_from_xml( TiXmlElement* el );

	sFacility* clone();
};

} // namespace WhoreMasterRenewal

#endif // SFACILITY_H_INCLUDED_1500
