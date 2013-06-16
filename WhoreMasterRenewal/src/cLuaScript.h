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
#ifndef CLUASCRIPT_H_INCLUDED_1523
#define CLUASCRIPT_H_INCLUDED_1523
#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <string>

namespace WhoreMasterRenewal
{

int get_from_table(lua_State *L, int table, const char *key, bool &dest);
int get_from_table(lua_State *L, int table, const char *key, int &dest);
bool get_from_table(lua_State *L, int table, const char *key, std::string &dest);


class cInterfaceWindowXML;
class Girl;

class cLuaStateInner
{
public:
	lua_State* L;

	void set_lua_path();
    static const std::string sandbox;
	
	cLuaStateInner();
	~cLuaStateInner();
	
	cLuaStateInner( const cLuaStateInner& ) = delete;
	cLuaStateInner& operator = ( const cLuaStateInner& ) = delete;
	
	inline operator lua_State* ()
	{
		return L;
	}
};

class cLuaState
{
public:
	cLuaState();
	
	inline operator lua_State* ()
	{
		return instance->L;
	}
	
private:
	static cLuaStateInner *instance;
};

class cLuaScript
{
public:
	cLuaScript();
	~cLuaScript();
	
	cLuaScript( const cLuaScript& ) = delete;
	cLuaScript& operator = ( const cLuaScript& ) = delete;
	
	void log_error();

	void set_param(const char *name, void *pointer);

	bool load(std::string filename, Girl *girl);

	bool run(const char *func = "script");
	bool process(cInterfaceWindowXML *window);
	bool refresh(cInterfaceWindowXML *window);
	bool call_handler(cInterfaceWindowXML *window, std::string handler_name);
	int get_ref(const char *name);
	bool run_by_ref(int ref);
	void set_wm_girl(Girl *girl);
	void set_wm_player();
	
private:
	cLuaState l;
    std::string m_file;
	bool running;
/*
 *	these are going to hold lua references
 *	pointers to the script's init() and run() methods
 */
	int init_ref;
	int run_ref;
/*
 *	the girl who is the subject (if any) of the event
 */
	Girl* girl;

    std::string slurp(std::string path);
	void get_param_table();
	bool get_from_space(const char *func);
};

} // namespace WhoreMasterRenewal

#endif // CLUASCRIPT_H_INCLUDED_1523
