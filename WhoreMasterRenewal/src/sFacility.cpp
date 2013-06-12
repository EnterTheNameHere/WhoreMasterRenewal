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

#include "sFacility.h"
#include "XmlMisc.h"
#include "CLog.h"
#include "Helper.hpp"

#include <cmath>

namespace WhoreMasterRenewal
{

/*
 *	defaults are set up for facility adjusters
 *	so range is 0-9, default to zero
 */
sBoundedVar::sBoundedVar()
{
    m_min = 0;
    m_max = 9;
    m_curr = 0;
}

/*
 *	but we could create one with any range
 */
sBoundedVar::sBoundedVar( int min, int max, int def )
{
    m_min = min;
    m_max = max;
    m_curr = def;
}

sBoundedVar::~sBoundedVar()
{
    
}

/*
 *	methods for adjuster buttons - simple increment with
 *	bounds checking
 */
void sBoundedVar::up()
{
    if( m_curr < m_max )
        m_curr++;
}
void sBoundedVar::down()
{
    if( m_curr > m_min )
        m_curr--;
}

/*
 *	operators = += -=
 */
sBoundedVar& sBoundedVar::operator =( int val )
{
    m_curr = val;
    bound();
    return *this;
}

sBoundedVar& sBoundedVar::operator +=( int val )
{
    m_curr += val;
    bound();
    return *this;
}

sBoundedVar& sBoundedVar::operator -=( int val )
{
    m_curr -= val;
    bound();
    return *this;
}

void sBoundedVar::bound()
{
    if( m_curr < m_min )
        m_curr = m_min;
    if( m_curr > m_max )
        m_curr = m_max;
}



/*
 *	we need to know how much extra space a bump would consume
 */
int sBoundedVar_Provides::space_needed()
{
/*
 *		we always use a whole space
 *		if we get 4 kennels per space, then we bump
 *		in bundles of 4 slots and one space
 */
    if( m_slots_per_space >= 1 )
    {
        return 1;
    }
/*
 *		otherwise we return the spaces needed for the
 *		next whole slot. Fractional slots are dropped
 */
    return int( floor( (m_curr + 1) / m_slots_per_space ) ) - m_space;
}

/*
 *	same exercise from a slot perspective
 */
int sBoundedVar_Provides::slots_needed()
{
    if(m_slots_per_space < 1)
    {
        return 1;
    }
    
    return int( ceil( (m_space + 1) * m_slots_per_space ) ) - m_curr;
}

void sBoundedVar_Provides::up()
{
    int slot_inc = slots_needed();
    if( m_curr + slot_inc > m_max )
    {
        return;
    }
    m_space += space_needed();
    m_curr += slot_inc;
}

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
void sBoundedVar_Provides::down()
{
    if( m_curr <= m_min )
    {
        return;
    }
/*
 *		The simplest case is if we get one slot per space
 *		this probably collapses elegantly into one of the
 *		other cases, but since I'm having trouble getting
 *		my head around the problem, I'm going to invoke the KISS principle.
 *		Keep It Simple, Stupid.
 */
    if( areEqual( m_slots_per_space, 1.0 ) )
    {
        m_curr --;
        m_space --;
        return;
    }
/*
 *		if the slots-per-space count is more than one
 *		we can just drop a space and re-calculate
 */
    if( m_slots_per_space > 1.0 )
    {
        m_space --;
/*
 *			just because we get more than one
 *			that doesn't mean we get a whole number
 *			it might be a 3-for-two deal, for instance
 *
 *			so we use floor to drop any fractional slots
 *			from the calculation
 */
        m_curr = int( floor(m_slots_per_space * m_space) );
        return;
    }
/*
 *		if we get here, we get less than one slot for a space
 *		so we drop the slot count by one, and then re-calculate space
 *		instead.
 */
    m_curr --;
/*
 *		again, we might not get a whole number - dorms use 6 slots
 *		in 2 spaces, for instance. So make sure the space requirement
 *		rounds UP
 */
    m_space = int( ceil(m_curr / m_slots_per_space) );
}

int sBoundedVar_Provides::bound()
{
    if(m_curr < m_min)
        m_curr = m_min;
    if(m_curr > m_max)
        m_curr = m_max;
/*
*		we're setting the slot count here,
*		so calculate the space based on that
*/
    m_space = int( ceil(m_curr / m_slots_per_space) );
    return m_curr;
}




sFacility::sFacility()
{
    m_type_name = "";
    m_instance_name = "";
    m_desc = "";
    m_space_taken = 0;
    m_slots = 0;
    m_base_price = 0;
    new_flag = false;
}

sFacility::sFacility( const sFacility& f )
{
    m_base_price = f.m_base_price;
    m_desc = f.m_desc;
    m_instance_name	= f.m_instance_name;
    m_slots = f.m_slots;
    m_space_taken = f.m_space_taken;
    m_type_name = f.m_type_name;
    new_flag = f.new_flag;
}

void sFacility::commit()
{
    new_flag = false;
    m_base_price = 0;
}

std::string sFacility::name()
{
    if(m_instance_name != "")
    {
        return m_instance_name;
    }
    return m_type_name;
}

std::string sFacility::desc()
{
    return m_desc;
}

std::string sFacility::type()
{
    return m_type_name;
}

int	sFacility::space_taken()
{
    return m_space_taken;
}

int	sFacility::slots()
{
    return m_slots;
}

int	sFacility::price()
{
    return tariff.buy_facility(m_base_price);
}

int	sFacility::glitz()
{
    return m_glitz.m_curr;
}

void sFacility::glitz_up()
{
    m_glitz.up();
}

void sFacility::glitz_down()
{
    m_glitz.down();
}

int	sFacility::secure()
{
    return m_secure.m_curr;
}

void sFacility::secure_up()
{
    m_secure.up();
}

void sFacility::secure_down()
{
    m_secure.down();
}

int	sFacility::stealth()
{
    return m_stealth.m_curr;
}

void sFacility::stealth_up()
{
    m_stealth.up();
}

void sFacility::stealth_down()
{
    m_stealth.down();
}

sFacility* sFacility::clone()
{
    return new sFacility(*this);
}

void sFacility::load_from_xml(TiXmlElement *base_el)
{
	XmlUtil u("Loading Facility Data from XML");

	new_flag = false;
	m_type_name = "Unknown";
	u.get_att(base_el, "Name", m_type_name);
	u.context(
		std::string("Loading Facility Data for ") +
		m_type_name +
		" from XML"
	);
	u.get_att(base_el,	"Instance",	m_instance_name, true);
	u.get_att(base_el,	"Space",	m_space_taken);
	u.get_att(base_el,	"Provides",	m_slots);
	u.get_att(base_el,	"Price",	m_base_price);


	TiXmlElement *el;
	for(	el = base_el->FirstChildElement();
		el ;
		el = el->NextSiblingElement()
	) {
	    std::string tag = el->ValueStr();

		if(tag != "BoundedVar") {
			g_LogFile.ss()	<< "Warning: Unexpected tag '"
				<< tag
				<< "': don't know what to do - ignoring."
			;
			g_LogFile.ssend();
			continue;
		}
/*
 *		we have (at the time of writing) three BoundedVar variables
 *		to parse: "Secure", "Stealth" and "Glitz"
 *
 *		the struct knows how to parse itself,
 *		but we need to find out which one it is
 */
	    std::string bvar_name;
		u.context(tag + " tag");
		u.get_att(el,	"Name",	bvar_name);
		g_LogFile.ss() << "loading boundedvar " << bvar_name << std::endl;
		g_LogFile.ssend();

		if(bvar_name == "Glitz") {
			m_glitz.from_xml(el);
		}
		else if(bvar_name == "Secure") {
			m_secure.from_xml(el);
		}
		else if(bvar_name == "Stealth") {
			m_stealth.from_xml(el);
		}
		else {
			g_LogFile.ss()	<< "Warning: unknown name '"
				<< bvar_name
				<< "' for bounded var"
			;
			g_LogFile.ssend();
		}
	}

}

TiXmlElement *sBoundedVar::to_xml(std::string name)
{
	TiXmlElement *el = new TiXmlElement("BoundedVar");
	el->SetAttribute("Name", name);
	el->SetAttribute("Min", m_min);
	el->SetAttribute("Max", m_max);
	el->SetAttribute("Curr", m_curr);
	return el;
}

bool sBoundedVar::from_xml(TiXmlElement *el)
{
	g_LogFile.ss() << "sBoundedVar::from_xml called";
	g_LogFile.ssend();
	XmlUtil u("Loading BoundedVar/Facility Data from XML");
/*
 *	The element we get passed should be a BoundedVar node
 *	so we just need to list the attributes
 */
	u.get_att(el, "Curr", m_curr);
	u.get_att(el, "Max", m_max);
	u.get_att(el, "Min", m_min);
	return true;
}

void sBoundedVar_Provides::init(sFacility *fac)
{
	m_min = fac->m_space_taken;
	m_max = 99;		// maybe set this in the XML
	m_curr = m_min;
	m_space = fac->m_space_taken;
/*
 *	we need to know how many slots per space
 */
	m_slots_per_space = 1.0 * m_curr / m_min;
}

} // namespace WhoreMasterRenewal
