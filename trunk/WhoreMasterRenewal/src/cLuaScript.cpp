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

#define LUA_COMPAT_MODULE

#include "cLuaScript.h"
#include "cLuaMenu.h"
#include "cScriptUtils.h"
#include "CLog.h"
#include "cMessageBox.h"
#include "cChoiceMessage.h"
#include "cBrothel.h"
#include "DirPath.h"
#include "FileList.h"
#include "sConfig.h"
#include "cInterfaceWindow.h"
#include "cWindowManager.h"
#include "cScriptManager.h"
#include "cTraits.h"
#include "cGirls.h"
#include "CGraphics.h"

#include <iostream>
#include <sstream>

extern CGraphics g_Graphics;
extern cChoiceManager g_ChoiceManager;
extern cMessageQue g_MessageQue;
extern cBrothelManager g_Brothels;
extern int g_CurrBrothel;
extern cWindowManager g_WinManager;
extern cInterfaceWindow g_BrothelManagement;
extern bool g_InitWin;
extern cGirls g_Girls;


typedef int (*lua_func)(lua_State *L);

static const char *stats[] = {
	"charisma",     "happiness",    "libido",
	"constitution", "intelligence", "confidence",
	"mana",         "agility",	"fame",
	"level",        "askprice",     "house",
	"exp",          "age",          "obedience",
	"spirit",	"beauty",       "tiredness",
	"health",       "pc_fear",	 "pc_love",
	"pc_hate",
	0
};

/*
 * same again for skill names
 */
static const char *skills[] =
{
	"anal",
	"magic",
	"bdsm",
	"normal",
	"beastiality",
	"group",
	"lesbian",
	"service",
	"strip",
	"combat",
	0
};

static void add_to_table(lua_State *L, int table, const char *key, lua_func f)
{
	lua_pushstring(L, key);
	lua_pushcfunction(L, f);
	lua_settable(L, table);
}

static void add_to_table(lua_State *L, int table, const char *key, char *value)
{
	lua_pushstring(L, key);
	lua_pushstring(L, value);
	lua_settable(L, table);
}

static void add_to_table(lua_State *L, int table, const char *key, int value)
{
	lua_pushstring(L, key);
	lua_pushinteger(L, value);
	lua_settable(L, table);
}

static void add_to_table(lua_State *L, int table, const char *key, std::string value)
{
	lua_pushstring(L, key);
	lua_pushstring(L, value.c_str());
	lua_settable(L, table);
}

static void add_to_table(lua_State *L, int table, const char *key, void *value)
{
	lua_pushstring(L, key);
	lua_pushlightuserdata (L, value);
	lua_settable(L, table);
}

/*
 * wm.sex {
	girl	= girl,		-- defaults to wm.girl
	girl2	= other_girl,	-- optional
 	type	= { "normal" | "anal" | "bdsm" | "beast" | "lesbian" },
	force	= { false | true },
 }
 */
// unused function
//static int sex(lua_State *L)
//{
//	CLog local_log;
// 	int argtab = lua_gettop(L);
// /*
//  *	make sure it exists and is in fact a table
//  */
//	if(argtab == 0) {
//		local_log.ss() << "Warning: missing parameter for wm.add_cust_to_brothel";
//		local_log.ssend();
//		return 1;
//	}
//	luaL_checktype(L, argtab, LUA_TTABLE);
//
//	//get_from_table(L, argtab, "girl", girl);
//	return 0;
//}

/*
 * usage is: dir1, dir2, dir3 ... dirN, pattern
 *
 * we'll take ./Resources as a given
 */
static int get_files(lua_State *L)
{
 	DirPath dp;
/*
 *	pop the pattern string off the end
 */
 	const char *pattern = lua_tostring(L, -1);
	if(!pattern) {
		g_LogFile.ss() << "Error: nil pattern!";
		g_LogFile.ssend();
		return 0;
	}
	lua_pop(L, 1);
/*
 *	now set up a DirPath
 */
 	dp << "Resources";
	int top = lua_gettop(L);
	for(int i = 0; i < top; i++) {
		dp << lua_tostring(L, i+1);
	}
	g_LogFile.ss() << "get_files: looking in '" << dp << "' for " << pattern;
	g_LogFile.ssend();
/*
 *	use the path and pattern to generate a list of matching files
 */
 	FileList fl(dp, pattern);
/*
 *	clear down the stack so we can use it to return the files we found
 */
	lua_settop(L, 0);
/*
 *	we need a table to hold all these
 */
 	lua_newtable(L);
/*
 *	loop through the files, pushing them onto the stack as we go
 */
	for(int i = 0; i < fl.size(); i++) {
		const char *pt = fl[i].full().c_str();

		g_LogFile.ss() << "	found: '" << pt << "'";
		g_LogFile.ssend();

		lua_pushinteger(L, i+1);
		lua_pushstring(L, pt);
		lua_settable(L, -3);
	}
	return 1;
}


static int menu_show(lua_State* /*L*/)
{
	cLuaMenu lmenu;

	lmenu.show();

	return 0;
}


static int get_sdl_ticks(lua_State* L)
{
	lua_pushinteger(L, g_Graphics.GetTicks());
	return 1;
}

static int pop_window(lua_State* /*L*/)
{
	g_LogFile.ss()
		<< "popping window stack"
	;
	g_LogFile.ssend();

	cWindowManager wman;
	wman.Pop();
	return 0;
}

bool get_from_table(lua_State *L, int table, const char *key, std::string &dest)
{
	lua_pushstring(L, key);
	lua_gettable(L, table);
/*
 *	did we find anything under that key?
 */
 	bool found = !lua_isnil(L, -1);
	if(found) {
		dest = lua_tostring(L, -1);
	}
/*
 *	pop the value (which may be nil) and return
 */
	lua_pop(L, 1);
	return found;
}

int get_from_table(lua_State *L, int table, const char *key, int &dest)
{
	lua_pushstring(L, key);
	lua_gettable(L, table);
/*
 *	did we find anything under that key?
 */
 	bool found = !lua_isnil(L, -1);
	if(found) {
		dest = lua_tointeger(L, -1);
	}
	lua_pop(L, 1);
/*
 *	pop the value (which may be nil) and return
 */
	return found;
}

int get_from_table(lua_State *L, int table, const char *key, bool &dest)
{
	lua_pushstring(L, key);
	lua_gettable(L, table);
/*
 *	did we find anything under that key?
 */
 	bool found = !lua_isnil(L, -1);
	if(found) {
		dest = lua_toboolean(L, -1);
	}
	lua_pop(L, 1);
/*
 *	pop the value (which may be nil) and return
 */
	return found;
}

/*
 * take a girl object and update the corresponding sGirl struct
 *
 * some use cases:
 *
 *	wm.girl:update()	-- updates wm.girl
 *	wm.girl.update()	-- updates wm.girl
 *	other_girl:update()	-- updates other_girl
 *
 * The one to watch out for will be
 *
 *	other_girl.update()	-- updates wm.girl
 *
 * which is a bit mental. Probably less hassle all around
 * if we just raise an error if the argument is nil
 */
static int update_girl(lua_State *L)
{
    std::string s;
	const char *pt;
	luaL_checktype(L, 1, LUA_TTABLE);
/*
 *	OK. the table is our lua girl object
 *	it should have a field called "pointer"
 */
 	int lua_girl = lua_gettop(L);
	lua_pushstring(L, "pointer");
	lua_gettable(L, lua_girl);
/*
 *	check for nil here. If there is no pointer,
 *	it's probably not a girl object
 */
 	if(lua_isnil(L, -1)) {
		return luaL_error(L,
			"girl.update: supplied object has no pointer"
		);
	}
	sGirl *girl = (sGirl *) lua_touserdata(L, -1);
/*
 *	set the stack back to the lua_girl
 */
 	lua_settop(L, lua_girl);
/*
 *	OK. let's look for differences with the version in memory
 *	and adjust as needed
 *
 *	There are probably good reasons to leave the name field alone
 *	so let's start with the real name
 */
	get_from_table(L, lua_girl, "real_name", s);
	if(s != girl->m_Realname) {
		girl->m_Realname = s;
	}
/*
 *	let's repeat that with the description. Be nice to have that
 *	evolve as the girl became accustomed to her new life
 */
	get_from_table(L, lua_girl, "desc", s);
	if(s != girl->m_Desc) {
		girl->m_Desc = s;
	}

/*
 *	next, we loop through the stats and check them
 */
	int lua_val, cpp_val;
	for(unsigned int i = 0; (pt = stats[i]); i++) {
		get_from_table(L, lua_girl, pt, lua_val);
		cpp_val = girl->get_stat(i);
/*
 *		skip it if there's no chance
 */
		int diff = lua_val - cpp_val;
 		if(diff == 0) {
			continue;
		}
		girl->upd_stat(i, diff);
	}
/*
 *	exact same thing with the skills
 */
	for(unsigned int i = 0; (pt = skills[i]); i++) {
		get_from_table(L, lua_girl, pt, lua_val);
		cpp_val = girl->get_skill(i);
/*
 *		skip it if there's no chance
 */
		int diff = lua_val - cpp_val;
 		if(diff == 0) {
			continue;
		}
		girl->upd_skill(i, diff);
	}
/*
 *	skip traits for now - I want to do them differently
 */
	return 0;
}

static int log_from_lua(lua_State *L)
{
	const char *msg = luaL_checkstring(L, 1);
    std::string s = "Lua Script: ";
	
	g_LogFile.write(s + msg);
	return 0;
}

static int game_over(lua_State* /*L*/)
{
	cScriptManager sm;

	g_MessageQue.AddToQue("GAME OVER", 1);
	g_WinManager.PopToWindow(&g_BrothelManagement);
	g_WinManager.Pop();
	g_InitWin = true;
	sm.Release();
	return 0;
}

static void add_trait_table(lua_State *L, sGirl *girl, int table)
{
/*
 *	push the string "traits" onto the stack - this is going to be the key
 *	when we add the trait table
 */
	lua_pushstring(L, "traits");
/*
 *	now we need a new empty table on the stack
 */
 	lua_newtable(L);
	int trait_table = lua_gettop(L);
/*
 *	now loop over the trait list
 */
	for(unsigned int i = 0; i < girl->m_NumTraits; i++) {
		sTrait *trait = girl->m_Traits[i];
/*
 *		store the description keyed under the trait name
 */
		add_to_table(L, trait_table, trait->m_Name, trait->m_Desc);
	}
/*
 *	and now add the trait table to the girl table
 */
 	lua_settable(L, table);
}

/*
 * really this needs a LuaGirl class and a lot more thought
 * for now, this is enough to add the girl to the current brothel
 */
static void make_lua_girl(lua_State *L, sGirl *girl)
{
	const char *pt;
/*
 *	If girl is null, push a nil on the stack and return
 *	that way we null out any pre-existing girl to show that
 *	there is no target girl for the current event
 */
	if(girl == 0) {
		lua_pushnil(L);
		return;
	}
/*
 *	OK, we don't get off that easy. We need an empty table, then.
 */
 	lua_newtable(L);
	int table = lua_gettop(L);
/*
 *	let's add the girl's name and so forth
 */
 	add_to_table(L, table, "name", girl->m_Name);
 	add_to_table(L, table, "real_name", girl->m_Realname);
 	add_to_table(L, table, "desc", girl->m_Desc);
/*
 *	let's add the sGirl pointer as light userdata
 */
 	add_to_table(L, table, "pointer", (void *)girl);
/*
 *	let's add the stats in, keyed by stat name in lower case
 */
	for(unsigned int i = 0; (pt = stats[i]); i++) {
		int val = girl->get_stat(i);
/*
 *		push the key, then they value
 */
		add_to_table(L, table, pt, val);
	}
/*
 *	same for skills
 */
	for(unsigned int i = 0; (pt = skills[i]); i++) {
		int val = girl->get_skill(i);
/*
 *		push the key, then they value
 */
		add_to_table(L, table, pt, val);
	}
/*
 *	add a subtable called traits containing the girl's traits
 */
	add_trait_table(L, girl, table);
/*
 *	add an update function
 */
	add_to_table(L, table, "update", update_girl);
/*
 *	and return leaving the new table on the stack
 */
 	return;
}

static int create_random_girl(lua_State *L)
{
	

	int age		= 17;
	bool global	= false;	// set to true to add her to the pool
	bool undead	= false;	// unused by CreateRandomGirl
	bool slave	= false;	// set to true to create her as a slave
	bool inhuman	= false;	// set to true for a non-human girl
					// really should allow "both"
	bool kidnapped	= false;	// set to true to create an abductee
/*
 *	now - let's have an arg table
 */
 	int argtab = lua_gettop(L);
/*
 *	make sure it exists and is in fact a table
 */
	if(argtab == 0) {
		g_LogFile.ss() << "Warning: missing parameter for wm.add_cust_to_brothel";
		g_LogFile.ssend();
		return 1;
	}
	luaL_checktype(L, argtab, LUA_TTABLE);
/*
 *	OK - let's pull some parameters out of that table
 *	defaults are retained if a parameter is omitted
 */
	get_from_table(L, argtab, "age", age);
	get_from_table(L, argtab, "global", global);
	get_from_table(L, argtab, "undead", undead);
	get_from_table(L, argtab, "slave",  slave);
	get_from_table(L, argtab, "inhuman",  inhuman);
	get_from_table(L, argtab, "kidnapped",  kidnapped);
/*
 *	now create the girl
 */
	sGirl *girl = g_Girls.CreateRandomGirl(
		age,		// age
		global,		// add to global girl list flag
		slave,		// create as slave flag
		undead,		// undead flag
		inhuman,	// true if non-humans are possible
		kidnapped	// true if kidnapped
	);
/*
 *	now create a lua table with the girl data
 */
	make_lua_girl(L, girl);
	return 1;
}

static int queue_message(lua_State *L)
{
	

	const char *msg = luaL_checkstring(L, 1);
	int color = luaL_checkint(L, 2);

	g_LogFile.ss() << "adding to message queue: '" << msg << "\n";
	g_LogFile.ss() << "Before add: has = " << g_MessageQue.HasNext();
	g_LogFile.ssend();

	g_MessageQue.AddToQue(msg, color);
	g_LogFile.ss() << "After add: has = " << g_MessageQue.HasNext();
	g_LogFile.ssend();
	return 0;
}

/*
 * expect an args table:
 *
 *	wm.add_cust_to_dungeon {
 *		reason		= "non-payment",
 *		wife		= true,			-- optional
 *		daughters	= 3,			-- optional
 *	}
 *
 * why do we need the wife and daughters? Well, the method that does the
 * actual adding creates a whole new customer to put in the dungeon.
 * and customers have wives and daughters, potentially, anyway.
 *
 * the wife and daughter count, I am therefore leaving up to the calling
 * script. I don't suppose they make much difference anyway.
 */
static int add_cust_to_brothel(lua_State *L)
{
	
/*
 *	OK - do we have a parameter?
 */
 	int argtab = lua_gettop(L);
/*
 *	if not, log a warning and return false
 */
	if(argtab == 0) {
		g_LogFile.ss() << "Warning: missing parameter for wm.add_cust_to_brothel";
		g_LogFile.ssend();
		return 1;
	}
	luaL_checktype(L, argtab, LUA_TTABLE);
/*
 *	OK. parameters currently recognised are
 *
 *	    std::string : reason
 *		bool	: wife
 *		int	: daughters
 */
 	lua_pushstring(L, "reason");
	lua_gettable(L, argtab);
	if(lua_isnil(L, -1)) {
		return luaL_error(L, "wm.add_cust_to_brothel: missing reason");
	}
    std::string reason = lua_tostring(L, -1);
	lua_pop(L, 1);
/*
 *	this would be a good place to check that string
 */
	int rval = 0;
	if(reason == "non-payment") {
		rval = DUNGEON_CUSTNOPAY;
	}
	else if(reason == "violence") {
		rval = DUNGEON_CUSTBEATGIRL;
	}
	else return luaL_error(L,
		"wm.add_cust_to_brothel: acceptable reason are "
		"'non-payment' and 'violence'"
	);
/*
 *	now the wife parameter this is optional, defaults to false
 */
 	lua_pushstring(L, "wife");
	lua_gettable(L, argtab);
	bool wife = false;
	if(!lua_isnil(L, -1)) {
		wife = lua_toboolean(L, -1);
	}
	lua_pop(L, 1);
/*
 *	same deal for daughters
 */
 	lua_pushstring(L, "daughters");
	lua_gettable(L, argtab);
	int daughters = 0;
	if(!lua_isnil(L, -1)) {
		daughters = lua_tointeger(L, -1);
	}
	lua_pop(L, 1);

	g_Brothels.GetDungeon()->AddCust(rval, daughters, wife);
	return 0;
}

static int add_girl_to_brothel(lua_State *L)
{
	
/*
 *	OK - do we have a parameter?
 */
 	int top = lua_gettop(L);
/*
 *	if not, log a warning and return false
 */
	if(top == 0) {
		g_LogFile.ss() << "Warning: missing parameter for wm.add_girl_to_brothel";
		g_LogFile.ssend();
		lua_pushnil(L);
		return 1;
	}
/*
 *	We expect the argument to be a girl object - so that's a table
 *	we need to look up the "pointer" field from that table
 */
 	int gtab = 1;
	lua_pushstring(L, "pointer");
	lua_gettable(L, gtab);
/*
 *	now convert that back to a pointer
 */
	sGirl *girl = (sGirl *) lua_touserdata(L, -1);
/*
 *	add the girl
 */
	cScriptUtils::add_girl_to_brothel(girl);
/*
 *	return true
 */
	lua_pushinteger(L, 1);
	return 1;
}

static const luaL_Reg funx [] = {
	{ "game_over", game_over },
	{ "create_random_girl", create_random_girl },
	{ "add_cust_to_brothel", add_cust_to_brothel },
	{ "add_girl_to_brothel", add_girl_to_brothel },
	{ "get_files", get_files },
	{ "menu", menu_show},
	{ "get_sdl_ticks", get_sdl_ticks },
	{ "pop_window", pop_window },
	{ "log", log_from_lua },
	{ "message", queue_message },
	{ 0, 0 }
};

cLuaStateInner::cLuaStateInner()
{
	L = luaL_newstate();
	luaL_openlibs(L);
/*
 *	this should put all the functions in the "wm" namespace.
 *	so: wm.message("foo!") and so on
 */

	luaL_register(L, "wm", funx);
	/*
	lua_register(L, "doscript", doscript);
	lua_register(L, "get_files", get_files);
	lua_register(L, "hide", hide);
	lua_register(L, "exit_game", exit_game);
	lua_register(L, "menu_show", menu_show);
	lua_register(L, "get_sdl_ticks", get_sdl_ticks);
	lua_register(L, "load_background", load_background);
	lua_register(L, "load_window", load_window);
	lua_register(L, "pop_window", pop_window);
	lua_register(L, "show", show);
	lua_register(L, "log", log_from_lua);
	lua_register(L, "message", queue_message);
	lua_register(L, "set_text_item", set_text_item);
	lua_register(L, "set_image_item", set_image_item);
	set_lua_path();
	*/
}

/*
 *
cLuaStateInner::set_param(const char *name, void *pt)
{
}
 */

cLuaStateInner *cLuaState::instance = 0;

std::string cLuaScript::slurp(std::string path)
{
	std::ifstream ifs(path.c_str(), std::ifstream::in);
	std::stringstream sstr;
	sstr << ifs.rdbuf();
	return sstr.str();
}

void cLuaScript::log_error()
{
	
    std::string errstr = lua_tostring(l, -1);
	lua_pop(l, 1);
	g_LogFile.ss()	<< "Script error in '"
			<< m_file
			<< "': "
			<< errstr
	;
	g_LogFile.ssend();
}

int cLuaScript::get_ref(const char *name)
{
/*
 *	OK - stick the name on the stack
 */
	lua_getglobal(l, name);
	if(lua_isnil(l, -1)) {
		g_LogFile.ss()
			<< "cLuaScript::get_ref - can't find "
			<< "'" << name << "'"
		;
		g_LogFile.ssend();
		return 0;
	}
/*
 *	turn it into a reference
 */
	int ref = luaL_ref(l, LUA_REGISTRYINDEX);
	g_LogFile.ss() << "stored '" << name << "' as reference " << ref;
	g_LogFile.ssend();
/*
 *	return the reference number
 */
 	return ref;
}

bool cLuaScript::load(std::string filename, sGirl *a_girl)
{
	int rc;
	m_file = filename;
	girl = a_girl;

	g_LogFile.ss() << "cLuaScript::load: " << filename;
	g_LogFile.ssend();
/*
 *	We need to open the file to make sure it exists and can be read
 *
 *	There doesn't seem to be a better, cross platform way
 *	to do this
 */
	std::ifstream ifs(m_file.c_str(), std::ifstream::in);
	bool ok = ifs.good();
	ifs.close();
	if(!ok) return false;
/*
 *	So: slurp the file
 */
    std::string prog = slurp(m_file);
/*
 *	load the string
 */
	rc = luaL_loadstring(l, prog.c_str());
/*
 *	a non-zero return code means an error occured
 *	log it, and then return false so the script manager
 *	can recycle the script
 */
	if(rc != 0) {
		log_error();
		return false;
	}
/*
 *	call the chunk
 */
	rc = lua_pcall(l, 0, 1, 0);
/*
 *	again, check for errors.
 */
	if(rc != 0) {
		log_error();
		return false;
	}
/*
 *	OK. the script should have defined two entry points:
 *	init() and run()
 *
 *	we need to find these, convert them into references, and store them
 */
 	init_ref = get_ref("init");
 	run_ref = get_ref("run");
	g_LogFile.ss() << "init_ref = " << init_ref << "\n";
	g_LogFile.ss() << "run_ref  = " << run_ref;
	g_LogFile.ssend();
/*
 *	check the return code
 */
	rc = lua_toboolean(l, -1);
	g_LogFile.ss() << "script ready: return value " << rc;
	g_LogFile.ssend();
	return (rc != 0);
}

/*
 * sort out the player data
 */
static void make_lua_player(lua_State *L, cPlayer *player)
{
	const char *pt;
/*
 *	We need an empty table.
 */
 	lua_newtable(L);
	int table = lua_gettop(L);
/*
 *	let's add the girl's name and so forth
 */
 	add_to_table(L, table, "win_flag", player->m_WinGame);
 	add_to_table(L, table, "suspicion", player->suspicion());
 	add_to_table(L, table, "disposition", player->disposition());
 	add_to_table(L, table, "customer_fear", player->customerfear());
/*
 *	let's add the stats in, keyed by stat name in lower case
 */
	for(unsigned int i = 0; (pt = stats[i]); i++) {
		int val = player->m_Stats[i];
/*
 *		push the key, then they value
 */
		add_to_table(L, table, pt, val);
	}
/*
 *	same for skills
 */
	for(unsigned int i = 0; (pt = skills[i]); i++) {
		int val = player->m_Skills[i];
/*
 *		push the key, then they value
 */
		add_to_table(L, table, pt, val);
	}
 	return;
}

/*
 * this sets the wm.girl field to the girl passed
 */
void cLuaScript::set_wm_girl(sGirl *girl)
{
/*
 *	get the "wm" symbol on the stack to start with
 */
 	lua_getglobal(l, "wm");
	//assert(lua_isnil(l, -1) == 0);
	int wm = lua_gettop(l);
	g_LogFile.ss() << "cLuaScript::set_wm_girl: wm index at " << wm;
	g_LogFile.ssend();
/*
 *	now push the key "girl" on after it
 */
 	lua_pushstring(l, "girl");
/*
 *	now format the sGirl data as a Lua table
 */
	make_lua_girl(l, girl);
/*
 *	last store the girl data in wm
 */
 	lua_settable(l, wm);
}

/*
 * this sets the wm.girl field to the girl passed
 */
void cLuaScript::set_wm_player()
{
	cPlayer *player = g_Brothels.GetPlayer();
/*
 *	get the "wm" symbol on the stack to start with
 */
 	lua_getglobal(l, "wm");
	int wm = lua_gettop(l);
/*
 *	now push the key "player" on after it
 */
 	lua_pushstring(l, "player");
/*
 *	now format the sGirl data as a Lua table
 */
	make_lua_player(l, player);
	//bool flag = !lua_isnil(l, -1);
	//assert(flag);
/*
 *	last store the data in wm
 */
 	lua_settable(l, wm);
}

bool cLuaScript::run(const char* /*func*/)
{
	bool ok;
/*
 *	let's start by making sure the stack is empty
 */
	lua_settop(l, 0);
/*
 *	the script should be loaded with the init and run routines
 *	stored by reference
 *
 *	check if the script is running. If not, run the init() func
 */
	if(!running) {
/*
 *		pack the target girl's data for lua
 */
		set_wm_girl(girl);
		set_wm_player();
/*
 *		rabbit rabbit rabbit rabbit
 *		rabbit rabbit rabbit rabbit
 *		yak yak yak yak yak
 */
		g_LogFile.ss() << "running init method";
		g_LogFile.ss() << "init_ref = " << init_ref << "\n";
		g_LogFile.ss() << "run_ref  = " << run_ref << "\n";
		g_LogFile.ssend();
/*
 *		run the init function
 */
		if(!run_by_ref(init_ref)) {
			g_LogFile.ss() << "script init method failed"
			;
			g_LogFile.ssend();
			return false;
		}
	}
	running = true;
	g_LogFile.ss() << "running run method";
	g_LogFile.ssend();
	ok = run_by_ref(run_ref);
	g_LogFile.ss() << "run method called: returns " << ok ;
	g_LogFile.ssend();
	return ok;
}

bool cLuaScript::run_by_ref(int ref)
{
	int rc;
	g_LogFile.ss() << "about to call reference: " << ref ;
	g_LogFile.ssend();
/*
 *	look up the funciton
 */
	lua_rawgeti(l, LUA_REGISTRYINDEX, ref);
/*
 *	and call it. We need the script func to return a bool
 *	so we can tell if it's done yet - so we need to reflect that
 *	in the parameters to lua_pcall:
 */
	rc = lua_pcall(
		l,		// Lua state
		0,		// number of args we're passing in
		1,		// number of results we expect returned
		0		// optional error handler
	);
/*
 *	check for errors
 */
	if(rc != 0) {
		log_error();
		return false;
	}
/*
 *	OK. If we get this far, Lua found the script func it
 *	executed without errors
 *
 *	Now we need to pull the return code off the stack
 */
	rc = lua_toboolean(l, -1);
/*
 *	if it returned zero, that means false was returned.
 *	Or it might mean that the coder forgot to return anything.
 *	or it might mean that nil was returned. In all these cases
 *	we return false meaning "remove me from the queue"
 */
	if(rc == 0) {
		return false;
	}
/*
 *	otherwise we assume the intention was "keep on going"
 */
	return true;
}


cLuaState::cLuaState()
{
    if(!instance)
        instance = new cLuaStateInner();
}


cLuaScript::cLuaScript()
{
    running = false;
    girl	= nullptr;
    init_ref= -1;
    run_ref	= -1;
}

cLuaScript::~cLuaScript()
{
    ;
}
