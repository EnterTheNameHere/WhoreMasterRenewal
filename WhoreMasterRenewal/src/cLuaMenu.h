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
#ifndef CLUAMENU_H_INCLUDED_1523
#define CLUAMENU_H_INCLUDED_1523
#pragma once

#include <string>
#include <vector>

namespace WhoreMasterRenewal
{

typedef std::vector<std::string*> str_vec;

// TODO: Add constructor
class cLuaMenuInner
{
public:
    void show();
    void clicked( int option_number );
    
private:
    /*
     *  parameters to the initial show method
     */
    int x = 0;
    int y = 0;
    int h = 0;
    int w = 0;
    int maxw = 0;
    int maxh = 0;
    int num_options = 0;
    std::string font_file = "Default cLuaMenuInner::font_file value";
    int font_size = 0;
    std::vector<std::string*> captions = {};
    int lua_callback_ref = 0;
    
    int get_int( const char *name, int def_val );
    std::string get_string( const char *name, const char *def_val );
    void whoops( std::string msg );
    int get_ref( const char * );
    str_vec get_caption_strings();
    str_vec traverse_caption_table();
    void calc_size_from_font( str_vec &v );
    void calc_co_ords( str_vec &v );
    int get_menu_x( int maxw );
    int get_menu_y( int maxh );
    void write_captions();
};

class cLuaMenu
{
public:
    cLuaMenu();
    
    void show();
    void clicked( int option_number );
    
private:
    static cLuaMenuInner* instance;
};

} // namespace WhoreMasterRenewal

#endif // CLUAMENU_H_INCLUDED_1523
