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
#ifndef CSCRIPTMANAGER_H_INCLUDED_1513
#define CSCRIPTMANAGER_H_INCLUDED_1513
#pragma once

#include "cGameScript.h" // required cGameScript

#include <queue>
#include <string>

class cLuaScript;
class ScriptPath;
struct sGirl;

/*
 * OK: I got into a bit of a mess the last time I tried this,
 * so this time I'm just going to wrap the current script manager
 * in another class
 *
 * let's see if we can get everything going through the wrapper
 * before we add the lua stuff in
 */
class cScriptManagerInner
{
public:
	void Load(std::string filename, sGirl* girl);
	void Release();
	bool IsActive();
	void RunScript();
	
private:
    cGameScript m_Script;
	std::queue<cLuaScript*> lq;
};

class cScriptManager
{
public:
	cScriptManager();
	void Load(ScriptPath& dp, sGirl* girl);
	void Load(std::string filename, sGirl* girl);
	void Release();
	bool IsActive();
	void RunScript();

private:
    static cScriptManagerInner* instance;
};

#endif // CSCRIPTMANAGER_H_INCLUDED_1513
