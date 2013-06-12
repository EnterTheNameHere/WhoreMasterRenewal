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
#ifndef CSCRIPTS_H_INCLUDED_1512
#define CSCRIPTS_H_INCLUDED_1512
#pragma once

#include <string>

namespace WhoreMasterRenewal
{

enum Types { _NONE = 0, _TEXT, _BOOL, _INT, _FLOAT, _CHOICE };
//typedef unsigned char bool;
//const unsigned int true = 1;
//const unsigned int false = 0;

typedef struct sEntry	// represents a single entry in an action
{
	unsigned int m_Type;	// Type of entry
	union
	{
		long m_NumChoices;	// how many choices in _CHOICE
		long m_TypeID;	// What id is _TYPE
		long m_lMin;	// min long number
		float m_fMin;	// min float number
	};

	union
	{
		long m_lMax;	// max long number
		float m_fMax;	// max float number
		char** m_Choices;	// text array of choices for _CHOICE
	};

	// Structure constructor to clear to default values
	sEntry();

	// Structure destructor to clean up used resources
	~sEntry();
}sEntry;

// Structure that stores a single action and contains
// a pointer for using linked lists.
typedef struct sAction
{
	long m_ID; // Action ID (0 to # actions-1)
	char m_Text[256]; // Action text
	short m_NumEntries; // # of entries in action
	sEntry* m_Entries; // Array of entry structures
	sAction* m_Next; // Next action in linked list

	sAction();
	~sAction();
	
} sAction;

typedef struct sScriptEntry
{
	long m_Type; // Type of entry (_TEXT, _BOOL, etc.)
	union
	{
		long m_IOValue; // Used for saving/loading
		long m_Length; // Length of text (w/ 0 terminator)
		long m_Selection; // Selection in choice
		bool m_bValue; // bool value
		long m_lValue; // long value
		float m_fValue; // float value
	};

	char* m_Text; // Text buffer
	unsigned char m_Var;

	sScriptEntry();
	~sScriptEntry();
	
} sScriptEntry;

typedef struct sScript
{
	long m_Type; // 0 to (number of actions-1)
	long m_NumEntries; // # entries in this script action
	sScriptEntry* m_Entries; // Array of entries
	sScript* m_Prev; // Prev in linked list
	sScript* m_Next; // Next in linked list

	sScript();
	~sScript();
	
} sScript;

class cActionTemplate
{
public:
	cActionTemplate();
	~cActionTemplate();

	// Load and free the action templates
	bool Load();
	bool Free();

	// Get # actions in template, action parent,
	// and specific action structure.
	long GetNumActions() {return m_NumActions;}
	sAction* GetActionParent(){return m_ActionParent;}
	sAction* GetAction(long Num);

	// Get a specific type of sScript structure
	sScript* CreateScriptAction(long Type);

	// Get info about actions and entries
	long GetNumEntries(long ActionNum);
	sEntry* GetEntry(long ActionNum, long EntryNum);

	// Expand action text using min/first/true choice values
	bool ExpandDefaultActionText(char* Buffer, sAction* Action);

	// Expand action text using selections
	bool ExpandActionText(char* Buffer, sScript* Script);
    
private:
	long m_NumActions; // # of actions in template
	sAction* m_ActionParent; // list of template actions

	// Functions for reading text (mainly used in actions)
	bool GetNextQuotedLine(char* Data, FILE* fp, long MaxSize);
	bool GetNextWord(char* Data, FILE* fp, long MaxSize);
};

// Class for processing scripts
class cScript
{
protected:
	long m_NumActions; // # of script actions loaded
	sScript* m_ScriptParent; // Script linked list

	// Overloadable functions for preparing for script
	// processing and when processing completed
	virtual bool Prepare() { return true; }
	virtual bool Release() { return true; }

	// Process a single script action
	virtual sScript* Process(sScript* Script) { return Script->m_Next; }

public:
	cScript();
	virtual ~cScript();

	bool Load(std::string filename); // Load a script
	bool Free(); // Free loaded script
/*
 *	no idea if this stub needs to return true or false
 *	picked one at random to silence a compiler warning
 */
	bool Execute(char* /*Filename=NULL*/) { /*Filename*/; return true; }; // Execute script
};

// General Functions
sScript* LoadScriptFile(std::string Filename);
bool SaveScriptFile(const char* Filename, sScript* ScriptRoot);
void TraverseScript(sScript* pScript);

} // namespace WhoreMasterRenewal

#endif // CSCRIPTS_H_INCLUDED_1512
