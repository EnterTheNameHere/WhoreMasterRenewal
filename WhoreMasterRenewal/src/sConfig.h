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
#ifndef SCONFIG_H_INCLUDED_1500
#define SCONFIG_H_INCLUDED_1500
#pragma once

#include <string>

class TiXmlElement;
struct SDL_Color;

namespace WhoreMasterRenewal
{

typedef unsigned int u_int;

struct sConfigData
{
    /*
     *  initialisation
     */
    struct
    {
        int gold = 4000;
        int girl_meet = 30;
        int slave_house_perc = 100;
        bool auto_use_items = false;
        int torture_mod = 1;
    } initial = {};
    /*
     *  income factors
     */
    
    struct InFactors
    {
        double extortion = 1.0;
        double brothel_work = 1.0;
        double street_work = 1.0;
        double movie_income = 1.0;
        double stripper_work = 1.0;
        double barmaid_work = 1.0;
        double slave_sales = 1.0;
        double item_sales = 1.0;
    } in_fact = {};
    /*
     *  outgoings factors
     */
    struct OutFactors
    {
        double training = 0.0;
        double actress_wages = 0.0;
        double movie_cost = 1.0;
        double goon_wages = 1.0;
        double matron_wages = 1.0;
        double staff_wages = 1.0; // TODO: Add it to xml config
        double girl_support = 1.0;
        double consumables = 1.0;
        double item_cost = 1.0;
        double slave_cost = 1.0;
        double brothel_cost = 1.0;
        double brothel_support = 1.0;
        double bar_cost = 1.0; // BarSupport in xml config
        double casino_cost = 1.0; // CasinoSupport in xml config
        double bribes = 1.0;
        double fines = 1.0;
        double advertising = 1.0;
    } out_fact = {};
    
    struct GambleData
    {
        int odds = 49;
        int base = 79;
        int spread = 400;
        double house_factor = 1.0;
        double customer_factor = 0.5;
    } gamble = {};
    
    struct TaxData
    {
        double rate = 0.06;
        double minimum = 0.01;
        double laundry = 0.25;
    } tax = {};
    
    struct PregnancyData
    {
        int player_chance = 8;
        int customer_chance = 8;
        int monster_chance = 8;
        double good_sex_factor = 8;
        int chance_of_girl = 50;
        int weeks_pregnant = 38;
        int weeks_till_grown = 60;
        int cool_down = 60;
    } pregnancy = {};
    
    struct GangData
    {
        int max_recruit_list = 6;
        int start_random = 2;
        int start_boosted = 2;
        int init_member_min = 1;
        int init_member_max = 10;
        int chance_remove_unwanted = 25;
        int add_new_weekly_min = 0;
        int add_new_weekly_max = 2;
    } gangs = {};
    
    struct ProstitutionData
    {
        double rape_streets = 5;
        double rape_brothel = 1;
    } prostitution = {};
    
    struct item_data
    {
        bool auto_combat_equip = true;
        SDL_Color* rarity_color[7];
    } items = {};
    
    struct font_data
    {
        std::string normal = "comic.ttf";
        std::string fixed = "comic.ttf";
        bool antialias = true;
    } fonts = {};
    
    struct DebugData
    {
        bool log_items = false;
        bool log_girls = false;
        bool log_rgirls = false;
        bool log_fonts = false;
        bool log_torture = false;
    } debug = {};
    
    std::string override_dir = "";
    
    sConfigData( const char *filename = "config.xml" );
    
    void set_defaults();
    void get_income_factors( TiXmlElement *el );
    void get_expense_factors( TiXmlElement *el );
    void get_tax_factors( TiXmlElement *el );
    void get_gambling_factors( TiXmlElement *el );
    void get_preg_factors( TiXmlElement *el );
    void get_gang_factors( TiXmlElement *el );
    void get_pros_factors( TiXmlElement *el );
    void get_item_data( TiXmlElement *el );
    void get_font_data( TiXmlElement *el );
    void get_initial_values( TiXmlElement *el );
    void get_att( TiXmlElement *el, const char *name, int *data );
    void get_att( TiXmlElement *el, const char *name, double *data );
    void get_att( TiXmlElement *el, const char *name, std::string &s );
    void get_att( TiXmlElement *el, const char *name, bool &bval );
    void get_debug_flags( TiXmlElement *el );
};

class cConfig
{
    static sConfigData *data;
public:
    cConfig();
    void reload( const char *filename );
    
    struct InFactors
    {
        double extortion()      { return data->in_fact.extortion; }
        double brothel_work()   { return data->in_fact.brothel_work; }
        double street_work()    { return data->in_fact.street_work; }
        double movie_income()   { return data->in_fact.movie_income; }
        double stripper_work()  { return data->in_fact.stripper_work; }
        double barmaid_work()   { return data->in_fact.barmaid_work; }
        double slave_sales()    { return data->in_fact.slave_sales; }
        double item_sales()     { return data->in_fact.item_sales; }
    } in_fact = {};
    /*
     *  outgoings factors
     */
    struct OutFactors
    {
        double training()        { return data->out_fact.training; }
        double actress_wages()   { return data->out_fact.actress_wages; }
        double movie_cost()      { return data->out_fact.movie_cost; }
        double goon_wages()      { return data->out_fact.goon_wages; }
        double matron_wages()    { return data->out_fact.matron_wages; }
        double staff_wages()     { return data->out_fact.staff_wages; }
        double girl_support()    { return data->out_fact.girl_support; }
        double consumables()     { return data->out_fact.consumables; }
        double item_cost()       { return data->out_fact.item_cost; }
        double slave_cost()      { return data->out_fact.slave_cost; }
        double brothel_cost()    { return data->out_fact.brothel_cost; }
        double brothel_support() { return data->out_fact.brothel_support; }
        double bar_cost()        { return data->out_fact.bar_cost; }
        double casino_cost()     { return data->out_fact.casino_cost; }
        double bribes()          { return data->out_fact.bribes; }
        double fines()           { return data->out_fact.fines; }
        double advertising()     { return data->out_fact.advertising; }
    } out_fact = {};
    
    struct ProstitutionData
    {
        double  rape_streets()  { return data->prostitution.rape_streets; }
        double  rape_brothel()  { return data->prostitution.rape_brothel; }
    } prostitution = {};
    
    struct font_data
    {
        std::string& normal()        { return data->fonts.normal; }
        std::string& fixed()         { return data->fonts.normal; }
        bool& antialias()            { return data->fonts.antialias; }
    } fonts = {};
    
    struct item_data
    {
        bool auto_combat_equip()            { return data->items.auto_combat_equip; }
        SDL_Color* rarity_color( int num )  { return data->items.rarity_color[num]; }
    } items = {};
    
    struct TaxData
    {
        double rate()          { return data->tax.rate; }
        double minimum()       { return data->tax.minimum; }
        double laundry()       { return data->tax.laundry; }
    } tax = {};
    
    struct GambleData
    {
        int odds()                  { return data->gamble.odds; }
        int base()                  { return data->gamble.base; }
        int spread()                { return data->gamble.spread; }
        double house_factor()       { return data->gamble.house_factor; }
        double customer_factor()    { return data->gamble.customer_factor; }
    } gamble = {};
    
    struct PregnancyData
    {
        int player_chance()         { return data->pregnancy.player_chance; }
        int customer_chance()       { return data->pregnancy.customer_chance; }
        int monster_chance()        { return data->pregnancy.monster_chance; }
        double good_sex_factor()    { return data->pregnancy.good_sex_factor; }
        int chance_of_girl()        { return data->pregnancy.chance_of_girl; }
        int weeks_pregnant()        { return data->pregnancy.weeks_pregnant; }
        int weeks_till_grown()      { return data->pregnancy.weeks_till_grown; }
        int cool_down()             { return data->pregnancy.cool_down; }
    } pregnancy = {};
    
    struct GangData
    {
        int max_recruit_list()       { return data->gangs.max_recruit_list; }
        int start_random()           { return data->gangs.start_random; }
        int start_boosted()          { return data->gangs.start_boosted; }
        int init_member_min()        { return data->gangs.init_member_min; }
        int init_member_max()        { return data->gangs.init_member_max; }
        int chance_remove_unwanted() { return data->gangs.chance_remove_unwanted; }
        int add_new_weekly_min()     { return data->gangs.add_new_weekly_min; }
        int add_new_weekly_max()     { return data->gangs.add_new_weekly_max; }
    } gangs = {};
    
    struct
    {
        int gold()                  { return data->initial.gold; }
        int girl_meet()             { return data->initial.girl_meet; }
        int slave_house_perc()      { return data->initial.slave_house_perc; }
        bool auto_use_items()       { return data->initial.auto_use_items; }
        int torture_mod()           { return data->initial.torture_mod; }
    } initial = {};
    
    struct
    {
        bool log_items()         { return data->debug.log_items; }
        bool log_rgirls()        { return data->debug.log_rgirls; }
        bool log_girls()         { return data->debug.log_girls; }
        bool log_fonts()         { return data->debug.log_fonts; }
        bool log_torture()       { return data->debug.log_torture; }
    } debug = {};
    
    std::string override_dir()
    {
        return data->override_dir;
    }
    
};

} // namespace WhoreMasterRenewal

#endif // SCONFIG_H_INCLUDED_1500
