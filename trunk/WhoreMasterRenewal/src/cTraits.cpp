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

#include "cTraits.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>

extern cTraits g_Traits;
cTraits g_Traits;

sTrait::sTrait()
{
    m_Name = m_Desc = 0;
    m_Next = 0;
}

sTrait::~sTrait()
{
    if(m_Name)
        delete [] m_Name;
    m_Name = 0;
    
    if(m_Desc)
        delete [] m_Desc;
    m_Desc = 0;

    if(m_Next)
        delete m_Next;
    m_Next = 0;
}

cTraits::cTraits(){m_ParentTrait=0;m_LastTrait=0;m_NumTraits=0;}

cTraits::~cTraits()
{
	Free();
}

void cTraits::Free()
{
	if(m_ParentTrait)
		delete m_ParentTrait;
	m_ParentTrait = m_LastTrait = 0;
}

void cTraits::LoadTraits(std::string filename)
{
	std::ifstream in;
	in.open(filename.c_str());
	char *pt, buffer[500];
	sTrait* newTrait = 0;

	while(in.good())
	{
		newTrait = new sTrait;

		if (in.peek()=='\n') in.ignore(1,'\n');
		in.getline(buffer, sizeof(buffer), '\n');		// get the name
		if((pt = strrchr(buffer, '\r'))) {
			*pt = 0;
		}

		newTrait->m_Name = new char[strlen(buffer)+1];
		strcpy(newTrait->m_Name, buffer);

		if (in.peek()=='\n') in.ignore(1,'\n');
		if (in.peek()=='\r') in.ignore(1,'\r');
		in.getline(buffer, sizeof(buffer), '\n');		// get the description
		if((pt = strrchr(buffer, '\r'))) {
			*pt = 0;
		}

		if(strcmp(buffer, "na") != 0)
		{
			newTrait->m_Desc = new char[strlen(buffer)+1];
			strcpy(newTrait->m_Desc, buffer);
		}
		else
			newTrait->m_Desc = 0;

		AddTrait(newTrait);
		newTrait = 0;
	}

	in.close();
}

void cTraits::SaveTraits(std::string filename)
{
	std::ofstream out;
	sTrait* current = m_ParentTrait;

	out.open(filename.c_str());

	while(current != 0)
	{
		out<<current->m_Name<<"\n";
		out<<current->m_Desc<<"\n";
		current = current->m_Next;
	}
	out.flush();
	out.close();
}

void cTraits::AddTrait(sTrait* trait)
{
	trait->m_Next = 0;
	if(m_ParentTrait)
		m_LastTrait->m_Next = trait;
	else
		m_ParentTrait = trait;
	m_LastTrait = trait;
	m_NumTraits++;
}

void cTraits::RemoveTrait(std::string name)
{
	if(m_ParentTrait == 0)
		return;

	sTrait* current = m_ParentTrait;
	sTrait* last = 0;

	if(strcmp(current->m_Name, name.c_str()) == 0)
	{
		m_ParentTrait = current->m_Next;
		current->m_Next = 0;
		delete current;
	}
	else
	{
		last = current ;
		current = current->m_Next;
		while(strcmp(current->m_Name, name.c_str()) == 0)
		{
			last = current ;
			current = current->m_Next;
		}

		last->m_Next = current->m_Next;

		current->m_Next = 0;
		delete current;
	}

	m_NumTraits--;
	current = last = 0;
}

sTrait* cTraits::GetTrait(std::string name)
{
	if(m_ParentTrait == 0)
		return 0;

	sTrait* current = m_ParentTrait;
/*
 *	MOD docclox
 *	changed the way the loop works to make it easier to include
 *	debug print statements
 */
	// g_LogFile.os() << "Looking up trait '" << name << "'" << std::endl;
	for(current = m_ParentTrait; current; current = current->m_Next) {
		/*
		g_LogFile.os()	<< "	testing '"
			<< current->m_Name
			<<"'"
			<< std::endl;
		*/
		if(name == current->m_Name) {
			//g_LogFile.os() << "		gotit!" << std::endl;
			return current;
		}
		//g_LogFile.os() << "		nope!" << std::endl;
	}
	// END MOD
	return 0;
}

sTrait* cTraits::GetTraitNum(int num)
{
	int count = 0;
	if(m_ParentTrait == 0)
		return 0;

	sTrait* current = m_ParentTrait;

	while(current)
	{
		if(count == num)
			break;
		count++;

		current = current->m_Next;
		if(current == 0)
			break;
	}

	return current;
}
