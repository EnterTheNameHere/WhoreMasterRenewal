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

#include "XmlUtil.h"
#include "XmlMisc.h"
#include "CLog.h"

bool XmlUtil::get_att( TiXmlElement* el, const char* name, int& ipt, bool optional )
{
	if(el->Attribute(name, &ipt) || optional) {
		return true;
	}
	g_LogFile.ss()	<< "Warning: " << m_context << ": No '"
		<< name
		<< "' attribute: defaulting to "
		<< ipt
	;
	g_LogFile.ssend();
	return false;
}

bool XmlUtil::get_att( TiXmlElement* el, const char* name, double& dpt, bool optional )
{
	if(el->Attribute(name, &dpt) || optional) {
		return true;
	}
	g_LogFile.ss()	<< "Warning: " << m_context << ": No '"
		<< name
		<< "' attribute: defaulting to "
		<< dpt
	;
	g_LogFile.ssend();
	return false;
}

bool XmlUtil::get_att( TiXmlElement* el, const char* name, bool& bval, bool optional )
{
	const char* pt;

	pt = el->Attribute(name);

	if(!pt) {
		if(optional) {
			return true;
		}
		g_LogFile.ss()	<< "Warning: " << m_context << ": No '"
			<< name
			<< "' attribute: defaulting to "
			<< bval
		;
		g_LogFile.ssend();
		return false;
	}


/*
 *      convert to a string, and then squash that to lower case
 */
    std::string s = pt;
	for(u_int i = 0; i < s.length(); ++i)
	{
		s[i] = tolower(s[i]);
	}
/*
 *      now we expect either "true" or "false"
 *      we can take "1" and "0" as well
 */
	if(s == "true" || s == "1") {
		bval = true;
		return true;
	}
	if(s == "false" || s == "0") {
		bval = false;
		return true;
	}
	g_LogFile.ss()	<< "Error: " << m_context << ": Unexpected value '"
		<< s
		<< "' in binary attribute '"
		<< name
		<< "'"
	;
	g_LogFile.ssend();
	return false;
}

bool XmlUtil::get_att( TiXmlElement* el, const char* name, std::string& s, bool optional )
{
	const char* pt;

	pt = el->Attribute(name);
	if(pt) {
		s = pt;
		return true;
	}
	if(optional) return true;
	g_LogFile.ss()	<< "Warning: " << m_context << ": No '"
		<< name
		<< "' attribute: defaulting to "
		<< s;
	g_LogFile.ssend();
	return false;
}

std::string& XmlUtil::context()
{
    return m_context;
}

void XmlUtil::context( std::string s )
{
    m_context = s;
}

XmlUtil::XmlUtil( std::string context )
{
    m_context = context;
}
