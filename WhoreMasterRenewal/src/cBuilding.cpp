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

#include "cBuilding.h"
#include "CLog.h"
#include "XmlMisc.h"
#include "sFacility.h"

namespace WhoreMasterRenewal
{

int cBuilding::capacity()
{
    return m_capacity;
}

int cBuilding::free_space()
{
    return m_free;
}

int cBuilding::used_space()
{
    return m_capacity - m_free;
}

cBuilding::cBuilding()
{
    m_capacity	= 20;
    m_free		= 20;
    m_reversion	= nullptr;
}

bool cBuilding::add(sFacility* fac)
{
    int needed = fac->space_taken();
    if(needed > m_free)
    {
        return false;
    }
    m_free -= needed;
    m_facilities.push_back(fac);
    return true;
}

sFacility* cBuilding::remove(int i)
{
    sFacility* fac = m_facilities[i];
    m_facilities.erase(m_facilities.begin()+i);
    return fac;
}

sFacility* cBuilding::item_at(int i)
{
    return m_facilities[i];
}

sFacility* cBuilding::operator[](int i)
{
    return item_at(i);
}

int	cBuilding::size()
{
    return m_facilities.size();
}

void cBuilding::commit()
{
    for(u_int i = 0; i < m_facilities.size(); i++) {
        m_facilities[i]->commit();
    }
}

void cBuilding::revert()
{
    for(u_int i = 0; i < m_facilities.size(); i++) {
        delete m_facilities[i];
    }
    m_facilities.clear();
    if(!m_reversion) {
        return;
    }
    m_free = m_capacity;
    for(u_int i = 0; i < m_reversion->size(); i++) {
        sFacility* fpt = (*m_reversion)[i];
        m_facilities.push_back(fpt);
        m_free -= fpt->space_taken();
    }
    delete m_reversion;
    m_reversion = nullptr;
}

std::ofstream& cBuilding::save(std::ofstream& ofs, std::string name)
{
    std::string s;
	std::stringstream ss;
/*
 *	we're going to build an XML structure in memory
 *	and then splurt it down the profferred ostream.
 *	(hostream? I been working on the program too long...)
 */
	TiXmlDocument doc;
/*
 *	I think that the decl is the <?xml ...> twaddle at the top
 *
 *	not sure how well this will play embedded in mid file.
 */
	TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );
/*
 *	document root node. In this case we'll use a <building> tag
 */
	TiXmlElement* root = new TiXmlElement("Building");
/*
 *	use an attribute to store the capacity
 *	we can re-create the used and free counts, but we
 *	need to save the capacity
 */
	root->SetAttribute("Capacity", m_capacity);
	doc.LinkEndChild( root );
/*
 *	let's add in a comment
 */
    root->SetAttribute("Name", name);
	doc.LinkEndChild( root );
	//TiXmlComment *comment = new TiXmlComment();
	//ss << "Building configuration for '" << name << "'";
	//comment->SetValue(ss.str().c_str());
	//root->LinkEndChild( comment );
/*
 *	Now we loop over all the rooms in the building
 */
	/*u_int lim = m_facilities.size();
 	for(u_int i = 0; i < lim; i++) {
		sFacility *fac = m_facilities[i];*/
/*
 *		OK: for each building, we need an element
 *		to store the data
 */
		/*TiXmlElement *el = new TiXmlElement("Facility");
		el->SetAttribute("Capacity",	m_capacity);
		el->SetAttribute("Name",	fac->m_type_name);
		el->SetAttribute("Instance",	fac->m_instance_name);
		el->SetAttribute("Space",	fac->m_space_taken);
		el->SetAttribute("Provides",	fac->m_slots);
		el->SetAttribute("Price",	fac->m_base_price);
		el->LinkEndChild(fac->m_glitz.to_xml("Glitz"));
		el->LinkEndChild(fac->m_secure.to_xml("Secure"));
		el->LinkEndChild(fac->m_stealth.to_xml("Stealth"));
		root->LinkEndChild( el );
	}*/

	ss.str("");
	ss << doc;
	s = ss.str();
	g_LogFile.write(s);
	ofs << s;
	return ofs;
}

std::ifstream& cBuilding::load(std::ifstream& ifs)
{
	TiXmlDocument doc;
	XmlUtil u("Loading building data from XML");
/*
 *	read the document
 */
	ifs >> doc;
	if(doc.Error()) {
		g_LogFile.ss() << "error loading building data from XML: " << std::endl;
		g_LogFile.ss()<< "Error: line " << doc.ErrorRow() << ", col " << doc.ErrorCol() << ": " << doc.ErrorDesc();
		g_LogFile.ssend();
		return ifs;
	}
	g_LogFile.ss() << "Loaded XML string: " << doc ;
	g_LogFile.ssend();
/*
 *	get the docuement root
 */
	TiXmlElement* root_el = doc.RootElement();
/*
 *	check the root for the capacity elemeent
 */
 	u.get_att(root_el, "Capacity", m_capacity);
	//u.get_att(root_el, "Name", ...);
/*
 *	loop over the elements attached to the root
 */
	/*m_free = m_capacity;
	for(	el = root_el->FirstChildElement();
		el ;
		el = el->NextSiblingElement()
	) {
		sFacility *fac = new sFacility();

		fac->load_from_xml(el);
		m_facilities.push_back(fac);
		m_free -= fac->space_taken();
	} */
	return ifs;
}

bool cBuilding::list_is_clean()
{
	u_int lim = m_facilities.size();
 	for(u_int i = 0; i < lim; i++) {
		sFacility *fac = m_facilities[i];

		if(fac->new_flag) {
			return false;
		}
	}
	return true;
}

void cBuilding::make_reversion_list()
{
/*
 *	clear out the old list if any
 *	(should be cleared by the buy or revert buttons)
 */
	if(m_reversion) {
		clear_reversion_list();
	}
	m_reversion = new vFacilities();

	u_int lim = m_facilities.size();
 	for(u_int i = 0; i < lim; i++) {
		sFacility *fac = m_facilities[i];

		m_reversion->push_back(
			new sFacility(*fac)
		);
	}
}

void cBuilding::clear_reversion_list()
{
	if(!m_reversion) {
		return;
	}
	vFacilities& list = (*m_reversion);
	u_int lim = list.size();
 	for(u_int i = 0; i < lim; i++) {
		delete list[i];
		list[i] = nullptr;
	}
	delete m_reversion;
	m_reversion = nullptr;
}

} // namespace WhoreMasterRenewal
