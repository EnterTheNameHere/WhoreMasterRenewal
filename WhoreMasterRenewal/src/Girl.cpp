#include "Girl.hpp"
#include "CLog.h"
#include "sConfig.h"
#include "cCustomers.h"
#include "cGangs.h"
#include "GangManager.hpp"
#include "BrothelManager.hpp"
#include "Brothel.hpp"
#include "cRng.h"
#include "XmlMisc.h"
#include "cTraits.h"
#include "GirlManager.hpp"

#include <cstring>

namespace WhoreMasterRenewal
{

char* n_strdup( const char* s );
char* n_strdup( const char* s )
{
    return strcpy( new char[strlen( s ) + 1], s );
}

bool Girl::m_maps_setup = false;
std::map<std::string, unsigned int> Girl::stat_lookup;
std::map<std::string, unsigned int> Girl::skill_lookup;
std::map<std::string, unsigned int> Girl::status_lookup;

const char* Girl::stat_names[] =
{

    "Charisma",
    "Happiness",
    "Libido",
    "Constitution",
    "Intelligence",
    "Confidence",
    "Mana",
    "Agility",
    "Fame",
    "Level",
    "AskPrice",
    "House",
    "Exp",
    "Age",
    "Obedience",
    "Spirit",
    "Beauty",
    "Tiredness",
    "Health",
    "PCFear",
    "PCLove",
    "PCHate"
};

/*
 * calculate the max like this, and it's self-maintaining
 */
const unsigned int Girl::max_stats = (
        sizeof( Girl::stat_names ) / sizeof( Girl::stat_names[0] )
                                     );
                                     
                                     
/*
 * same again for skill names
 */
const char* Girl::skill_names[] =
{
    "Anal",
    "Magic",
    "BDSM",
    "NormalSex",
    "Beastiality",
    "Group",
    "Lesbian",
    "Service",
    "Strip",
    "Combat"
};

const unsigned int Girl::max_skills = (
        sizeof( Girl::skill_names ) / sizeof( Girl::skill_names[0] )
                                      );
                                      
const char* Girl::status_names[] =
{
    "None",
    "Poisoned",
    "Badly Poisoned",
    "Pregnant",
    "Pregnant By Player",
    "Slave",
    "Has Daughter",
    "Has Son",
    "Inseminated",
    "Controlled",
    "Catacombs"
};

const unsigned int Girl::max_statuses = (
        sizeof( Girl::status_names ) / sizeof( Girl::status_names[0] )
                                        );
                                        
// ----- Lookups
void Girl::setup_maps()
{

    //if(m_maps_setup)
    //  return; // only need to do this once
    
    g_LogFile.ss() << "[Girl::setup_maps] Setting up Stats, Skills and Status codes." << std::endl;
    g_LogFile.ssend();
    
    m_maps_setup = true;
    stat_lookup["Charisma"]     = STAT_CHARISMA;
    stat_lookup["Happiness"]    = STAT_HAPPINESS;
    stat_lookup["Libido"]       = STAT_LIBIDO;
    stat_lookup["Constitution"] = STAT_CONSTITUTION;
    stat_lookup["Intelligence"] = STAT_INTELLIGENCE;
    stat_lookup["Confidence"]   = STAT_CONFIDENCE;
    stat_lookup["Mana"]         = STAT_MANA;
    stat_lookup["Agility"]      = STAT_AGILITY;
    stat_lookup["Fame"]         = STAT_FAME;
    stat_lookup["Level"]        = STAT_LEVEL;
    stat_lookup["AskPrice"]     = STAT_ASKPRICE;
    stat_lookup["House"]        = STAT_HOUSE;
    stat_lookup["Exp"]          = STAT_EXP;
    stat_lookup["Age"]          = STAT_AGE;
    stat_lookup["Obedience"]    = STAT_OBEDIENCE;
    stat_lookup["Spirit"]       = STAT_SPIRIT;
    stat_lookup["Beauty"]       = STAT_BEAUTY;
    stat_lookup["Tiredness"]    = STAT_TIREDNESS;
    stat_lookup["Health"]       = STAT_HEALTH;
    stat_lookup["PCFear"]       = STAT_PCFEAR;
    stat_lookup["PCLove"]       = STAT_PCLOVE;
    stat_lookup["PCHate"]       = STAT_PCHATE;
    
    skill_lookup["Anal"]        = SKILL_ANAL;
    skill_lookup["Magic"]       = SKILL_MAGIC;
    skill_lookup["BDSM"]        = SKILL_BDSM;
    skill_lookup["NormalSex"]   = SKILL_NORMALSEX;
    skill_lookup["Beastiality"] = SKILL_BEASTIALITY;
    skill_lookup["Group"]       = SKILL_GROUP;
    skill_lookup["Lesbian"]     = SKILL_LESBIAN;
    skill_lookup["Service"]     = SKILL_SERVICE;
    skill_lookup["Strip"]       = SKILL_STRIP;
    skill_lookup["Combat"]      = SKILL_COMBAT;
    
    //  WD: Missing mapping for status
    status_lookup["None"]               = STATUS_NONE;
    status_lookup["Poisoned"]           = STATUS_POISONED;
    status_lookup["Badly Poisoned"]     = STATUS_BADLY_POISONED;
    status_lookup["Pregnant"]           = STATUS_PREGNANT;
    status_lookup["Pregnant By Player"] = STATUS_PREGNANT_BY_PLAYER;
    status_lookup["Slave"]              = STATUS_SLAVE;
    status_lookup["Has Daughter"]       = STATUS_HAS_DAUGHTER;
    status_lookup["Has Son"]            = STATUS_HAS_SON;
    status_lookup["Inseminated"]        = STATUS_INSEMINATED;
    status_lookup["Controlled"]         = STATUS_CONTROLLED;
    status_lookup["Catacombs"]          = STATUS_CATACOMBS;
    
}

int Girl::lookup_skill_code( std::string s )
{
    /*
     *  be useful to be able to log unrecognised
     *  type names here
     */
    if( skill_lookup.find( s ) == skill_lookup.end() )
    {
        g_LogFile.ss() << "[Girl::lookup_skill_code] Error: unknown Skill: " <<
                       s << std::endl;
        g_LogFile.ssend();
        return -1;
    }
    
    return skill_lookup[s];
}

int Girl::lookup_status_code( std::string s )
{
    /*
     *  be useful to be able to log unrecognised
     *  type names here
     */
    if( status_lookup.find( s ) == status_lookup.end() )
    {
        g_LogFile.ss() << "[Girl::lookup_status_code] Error: unknown Status: " <<
                       s << std::endl;
        g_LogFile.ssend();
        return -1;
    }
    
    return status_lookup[s];
}

int Girl::lookup_stat_code( std::string s )
{
    /*
     *  be useful to be able to log unrecognised
     *  type names here
     */
    if( stat_lookup.find( s ) == stat_lookup.end() )
    {
        g_LogFile.ss() << "[Girl::lookup_stat_code] Error: unknown Stat: " <<
                       s << std::endl;
        g_LogFile.ssend();
        return -1;
    }
    
    return stat_lookup[s];
}

Girl::Girl()
{
    m_Stats[STAT_HOUSE] = 60;
    
    for( int i = 0; i < 40; i++ )
    {
        m_EquipedItems[i]   = 0;
        m_Inventory[i]      = nullptr;
    }
    
    for( int i = 0; i < MAXNUM_TRAITS; i++ )
    {
        m_Traits[i]         = nullptr;
        m_TempTrait[i]      = 0;
    }
    
    for( int i = 0; i < MAXNUM_TRAITS * 2; i++ )
        m_RememTraits[i]    = nullptr;
        
    for( int i = 0; i < NUM_GIRLFLAGS; i++ )
        m_Flags[i]          = 0;
        
    for( u_int i = 0; i < NUM_SKILLS; i++ )
        m_TempSkills[i] = m_SkillMods[i] = 0;
        
    for( int i = 0; i < NUM_STATS; i++ )
        m_TempStats[i] = m_StatMods[i] = 0;
        
    for( u_int i = 0; i < NUM_ACTIONTYPES; i++ )
        m_Enjoyment[i] = -20;   // start off disliking everything
        
    /*
     *      MOD: DocClox, Sun Nov 15 06:08:32 GMT 2009
     *      initialise maps to look up stat and skill names
     *      needed for XML loader
     *
     *      things that need to happen every time the struct
     *      is constructed need to go before this point
     *      or they'll only happen the first time around
     */
    if( !m_maps_setup ) // only need to do this once
        setup_maps();
}

Girl::~Girl()
{
    m_GirlImages = nullptr;
    
    if( m_Name )
        delete [] m_Name;
        
    m_Name = nullptr;
    
    m_Events.Free();
    
    if( m_Next )
        delete m_Next;
        
    m_Next = nullptr;
    m_Prev = nullptr;
}

int Girl::get_stat( int stat_id )
{
    return g_GirlsPtr->GetStat( this, stat_id );
}

int Girl::upd_temp_stat( int stat_id, int amount )
{
    g_GirlsPtr->UpdateTempStat( this, stat_id, amount );
    return g_GirlsPtr->GetStat( this, stat_id );
}

int Girl::upd_stat( int stat_id, int amount )
{
    g_GirlsPtr->UpdateStat( this, stat_id, amount );
    return g_GirlsPtr->GetStat( this, stat_id );
}

int Girl::get_skill( int skill_id )
{
    return g_GirlsPtr->GetSkill( this, skill_id );
}

int Girl::upd_temp_skill( int skill_id, int amount )
{
    g_GirlsPtr->UpdateTempSkill( this, skill_id, amount );
    return g_GirlsPtr->GetSkill( this, skill_id );
}

int Girl::upd_skill( int skill_id, int amount )
{
    g_GirlsPtr->UpdateSkill( this, skill_id, amount );
    return g_GirlsPtr->GetSkill( this, skill_id );
}

/* UNUSED
bool Girl::carrying_monster()
{
    return( m_States & ( 1 << STATUS_INSEMINATED ) ) != 0;
}
*/

bool Girl::carrying_human()
{
    return carrying_players_child() || carrying_customer_child();
}

bool Girl::carrying_players_child()
{
    return( m_States & ( 1 << STATUS_PREGNANT_BY_PLAYER ) ) != 0;
}

bool Girl::carrying_customer_child()
{
    return( m_States & ( 1 << STATUS_PREGNANT ) ) != 0;
}

bool Girl::is_pregnant()
{
    return( m_States & ( 1 << STATUS_PREGNANT ) ||
            m_States & ( 1 << STATUS_PREGNANT_BY_PLAYER ) ||
            m_States & ( 1 << STATUS_INSEMINATED )
          );
}

bool Girl::is_mother()
{
    return( m_States & ( 1 << STATUS_HAS_DAUGHTER )
            || m_States & ( 1 << STATUS_HAS_SON ) );
}

bool Girl::is_poisoned()
{
    return( m_States & ( 1 << STATUS_POISONED )
            || m_States & ( 1 << STATUS_BADLY_POISONED ) );
}

void Girl::clear_pregnancy()
{
    m_States &= ~( 1 << STATUS_PREGNANT );
    m_States &= ~( 1 << STATUS_PREGNANT_BY_PLAYER );
    m_States &= ~( 1 << STATUS_INSEMINATED );
}

void Girl::add_trait( std::string trait, bool temp )
{
    g_GirlsPtr->AddTrait( this, trait, temp );
}

bool Girl::has_trait( std::string trait )
{
    return g_GirlsPtr->HasTrait( this, trait );
}

bool Girl::is_addict()
{
    return  has_trait( "Shroud Addict" )  ||
            has_trait( "Fairy Dust Addict" )  ||
            has_trait( "Viras Blood Addict" )
            ;
}

sChild* Girl::next_child( sChild* child, bool remove )
{
    if( !remove )
    {
        return child->m_Next;
    }
    
    return m_Children.remove_child( child, this );
}

/* UNUSED
int Girl::preg_type( int image_type )
{
    int new_type = image_type + PREG_OFFSET;
    
    //
    //      if the new image type is >=  NUM_IMGTYPES
    //      then it was one of the types that doesn't have
    //      an equivalent pregnant form
    //
    if( new_type >= NUM_IMGTYPES )
    {
        return image_type;
    }
    
    return new_type;
}
*/

bool Girl::is_slave()
{
    return ( m_States & ( 1 << STATUS_SLAVE ) ) != 0;
}

/* UNUSED
bool Girl::is_free()
{
    return !is_slave();
}
*/

void Girl::set_slave()
{
    m_States |= ( 1 << STATUS_SLAVE );
}

bool Girl::is_monster()
{
    return ( m_States & ( 1 << STATUS_CATACOMBS ) ) != 0;
}

/* UNUSED
bool Girl::is_human()
{
    return !is_monster();
}
*/

int Girl::preg_chance( int base_pc, bool good, double factor )
{
    cConfig cfg;
    g_LogFile.ss() << "Girl::preg_chance: " << "base %chance = " << base_pc << ", " << "good flag = " << good << ", "
                   << "factor = " << factor;
    g_LogFile.ssend();
    
    double chance = base_pc;
    /*
     *  factor is used to pass situational modifiers.
     *  For instance BDSM has a 25% reduction in chance
     */
    chance *= factor;
    
    /*
     *  if the sex was good, the chance is modded, again from the
     *  config file
     */
    if( good )
    {
        chance *= cfg.pregnancy.good_sex_factor();
    }
    
    return int( chance );
}

bool Girl::calc_pregnancy( cPlayer* player, bool good, double factor )
{
    cConfig cfg;
    double chance = preg_chance(
                        cfg.pregnancy.player_chance(), good, factor
                    );
    /*
     *  now do the calculation
     */
    return g_GirlsPtr->CalcPregnancy(
               this, int( chance ), STATUS_PREGNANT_BY_PLAYER,
               player->m_Stats,
               player->m_Skills
           );
}

bool Girl::calc_pregnancy( sCustomer* cust, bool good, double factor )
{
    cConfig cfg;
    double chance = preg_chance(
                        cfg.pregnancy.customer_chance(), good, factor
                    );
    /*
     *  now do the calculation
     */
    return g_GirlsPtr->CalcPregnancy(
               this,
               int( chance ),
               STATUS_PREGNANT,
               cust->m_Stats,
               cust->m_Skills
           );
}

bool Girl::calc_insemination( sCustomer* cust, bool good, double factor )
{
    cConfig cfg;
    double chance = preg_chance(
                        cfg.pregnancy.monster_chance(), good, factor
                    );
    /*
     *  now do the calculation
     */
    return g_GirlsPtr->CalcPregnancy(
               this,
               int( chance ),
               STATUS_INSEMINATED,
               cust->m_Stats,
               cust->m_Skills
           );
}

bool Girl::calc_insemination( cPlayer* player, bool good, double factor )
{
    cConfig cfg;
    double chance = preg_chance(
                        cfg.pregnancy.monster_chance(), good, factor
                    );
    /*
     *  now do the calculation
     */
    return g_GirlsPtr->CalcPregnancy(
               this,
               int( chance ),
               STATUS_INSEMINATED,
               player->m_Stats,
               player->m_Skills
           );
}

void Girl::OutputGirlRow( std::string* Data, const std::vector<std::string>& columnNames )
{
    for( unsigned int x = 0; x < columnNames.size(); ++x )
    {
        //for each column, write out the statistic that goes in it
        OutputGirlDetailString( Data[x], columnNames[x] );
    }
}

void Girl::OutputGirlDetailString( std::string& Data, const std::string& detailName )
{
    //given a statistic name, set a string to a value that represents that statistic
    static std::stringstream ss;
    ss.str( "" );
    
    if( detailName == "Name" )
    {
        ss << m_Realname;
    }
    else if( detailName == "Health" )
    {
        if( get_stat( STAT_HEALTH ) == 0 )
            ss << "DEAD";
        else
            ss << get_stat( STAT_HEALTH ) << "%";
    }
    else if( detailName == "Looks" )
    {
        ss << ( ( get_stat( STAT_BEAUTY ) + get_stat( STAT_CHARISMA ) ) / 2 );
        ss << "%";
    }
    else if( detailName == "Tiredness" )
    {
        ss << get_stat( STAT_TIREDNESS ) << "%";
    }
    else if( detailName == "Happiness" )
    {
        ss << get_stat( STAT_HAPPINESS ) << "%";
    }
    else if( detailName == "Age" )
    {
        if( get_stat( STAT_AGE ) == 100 )
        {
            ss << "???";
        }
        else
        {
            ss << get_stat( STAT_AGE );
        }
    }
    else if( detailName == "Virgin" )
    {
        if( m_Virgin )
        {
            ss << "Yes";
        }
        else
        {
            ss << "No";
        }
    }
    else if( detailName == "Weeks_Due" )
    {
        if( is_pregnant() )
        {
            cConfig cfg;
            int to_go = cfg.pregnancy.weeks_pregnant() - m_WeeksPreg;
            ss << to_go;
        }
        else
        {
            ss << "---";
        }
    }
    else if( detailName == "PregCooldown" )
    {
        ss << static_cast<int>( m_PregCooldown );
    }
    else if( detailName == "Accomodation" )
    {
        if( m_AccLevel == 0 )
            ss << "Very Poor";
        else if( m_AccLevel == 1 )
            ss << "Adequate";
        else if( m_AccLevel == 2 )
            ss << "Nice";
        else if( m_AccLevel == 3 )
            ss << "Good";
        else if( m_AccLevel == 4 )
            ss << "Wonderful";
        else if( m_AccLevel == 5 )
            ss << "High Class";
        else
        {
            ss << "Error";
        }
    }
    else if( detailName == "Gold" )
    {
        if( g_Gangs.GetGangOnMission( MISS_SPYGIRLS ) )
        {
            ss << m_Money;
        }
        else
        {
            ss << "???";
        }
    }
    else if( detailName == "DayJob" )
    {
        if( m_DayJob >= NUM_JOBS )
        {
            ss << "None";
        }
        else
        {
            ss << g_Brothels.m_JobManager.JobName[m_DayJob];
        }
    }
    else if( detailName == "NightJob" )
    {
        if( m_NightJob >= NUM_JOBS )
        {
            ss << "None";
        }
        else
        {
            ss << g_Brothels.m_JobManager.JobName[m_NightJob];
        }
    }
    else if( detailName.find( "STAT_" ) != std::string::npos )
    {
        std::string stat = detailName;
        stat.replace( 0, 5, "" );
        int code = Girl::lookup_stat_code( stat );
        
        if( code != -1 )
        {
            ss << get_stat( code );
        }
        else
        {
            ss << "Error";
        }
    }
    else if( detailName.find( "SKILL_" ) != std::string::npos )
    {
        std::string skill = detailName;
        skill.replace( 0, 6, "" );
        int code = Girl::lookup_skill_code( skill );
        
        if( code != -1 )
        {
            ss << get_skill( code );
        }
        else
        {
            ss << "Error";
        }
    }
    else if( detailName.find( "STATUS_" ) != std::string::npos )
    {
        std::string status = detailName;
        status.replace( 0, 7, "" );
        int code = lookup_status_code( status );
        
        if( code != -1 )
        {
            if( m_States & ( 1 << code ) )
            {
                ss << "Yes";
            }
            else
            {
                ss << "No";
            }
        }
        else
        {
            ss << "Error";
        }
    }
    else if( detailName == "is_slave" )
    {
        if( is_slave() )
        {
            ss << "Yes";
        }
        else
        {
            ss << "No";
        }
    }
    else if( detailName == "carrying_human" )
    {
        if( carrying_human() )
        {
            ss << "Yes";
        }
        else
        {
            ss << "No";
        }
    }
    else if( detailName == "is_pregnant" )
    {
        if( is_pregnant() )
        {
            ss << "Yes";
        }
        else
        {
            ss << "No";
        }
    }
    else if( detailName == "is_addict" )
    {
        if( is_addict() )
        {
            ss << "Yes";
        }
        else
        {
            ss << "No";
        }
    }
    else if( detailName == "is_mother" )
    {
        if( is_mother() )
        {
            ss << "Yes";
        }
        else
        {
            ss << "No";
        }
    }
    else if( detailName == "is_poisoned" )
    {
        if( is_poisoned() )
        {
            ss << "Yes";
        }
        else
        {
            ss << "No";
        }
    }
    else
    {
        ss << "Not found";
    }
    
    Data = ss.str();
}

bool Girl::calc_pregnancy( int chance, cPlayer* player )
{
    return g_GirlsPtr->CalcPregnancy(
               this,
               chance,
               STATUS_PREGNANT_BY_PLAYER,
               player->m_Stats,
               player->m_Skills
           );
}

Girl* Girl::run_away()
{
    m_RunAway = 6;      // player has 6 weeks to retreive
    m_NightJob = m_DayJob = JOB_RUNAWAY;
    g_Brothels.AddGirlToRunaways( this );
    return nullptr;
}

/*
 * passing the girl_wins boolean here to avoid confusion over
 * what returning TRUE actually means
 */

// MYR: How should incorporial factor in to this?

void Girl::fight_own_gang( bool& girl_wins )
{
    girl_wins = false;
    std::vector<sGang*> v = g_Gangs.gangs_on_mission( MISS_GUARDING );
    /*
     *  we'll take goons from the top gang in the list
     */
    sGang* gang = v[0];
    /*
     *  4 + 1 for each gang on guard duty
     *  that way there's a benefit to multiple gangs guarding
     */
    int max_goons = 4 + v.size();
    
    /*
     *  to the maximum of the number in the gang
     */
    if( max_goons > gang->m_Num )
    {
        max_goons = gang->m_Num;
    }
    
    /*
     *  now - sum the girl and gang stats
     *  we're not going to average the gangs.
     *  yes this gives them an unfair advantage
     *  that's the point of having 5:1 odds :)
     */
    int girl_stats = combat() + magic() + intelligence();
    /*
     *  Now the gangs. I'm not factoring the girl's health
     *  because there's something dramatically satisfying
     *  about her breeaking out of the dungeon after being
     *  tortured near unto death, and then still beating the
     *  thugs up. You'd buy into it in a Hollywood blockbuster...
     *
     *  Annnnyway....
     */
    int goon_stats = 0;
    
    for( int i = 0; i < max_goons; i++ )
    {
        goon_stats +=   gang->combat() +
                        gang->magic() +
                        gang->intelligence()
                        ;
    }
    
    /*
     *  the girl's base chance of winning is determined by the stat ratio
     */
    double odds = 1.0 * girl_stats / ( goon_stats + girl_stats );
    
    /*
     *  let's add some trait based bonuses
     *  I'm not going to do any that are already reflected in stat values
     *  (so no "Psychic" bonus, no "Tough" either)
     *  we can streamline this with the trait overhaul
     */
    if( has_trait( "Clumsy" ) )     odds -= 0.05;
    
    if( has_trait( "Broken Will" ) )    odds -= 0.10;
    
    if( has_trait( "Meek" ) )       odds -= 0.05;
    
    if( has_trait( "Dependant" ) )  odds -= 0.10;
    
    if( has_trait( "Fearless" ) )   odds += 0.10;
    
    if( has_trait( "Fleet of Foot" ) )  odds += 0.10;
    
    /*
     *  get it back into the 0 <= N <= 1 range
     */
    if( odds < 0 ) odds = 0;
    
    if( odds > 1 ) odds = 1;
    
    /*
     *  roll the dice! If it passes then the girl wins
     */
    if( g_Dice.percent( int( odds * 100 ) ) )
    {
        win_vs_own_gang( v, max_goons, girl_wins );
    }
    else
    {
        lose_vs_own_gang( v, max_goons, girl_stats, goon_stats, girl_wins );
    }
}

void Girl::win_vs_own_gang( std::vector<sGang*>& v, int max_goons, bool& girl_wins )
{
    sGang* gang = v[0];
    girl_wins = true;
    /*
     *  Give her some damage from the combat. She won, so don't kill her.
     *  should really modify this for traits. And agility.
     */
    int damage = g_Dice.random( 10 ) * max_goons;
    health( -damage );
    
    if( health() == 0 ) health( 1 );
    
    /*
     *  now - how many goons die today?
     *  I'm assuming the girl's priority is to escape
     *  rather than "clearing the level" so let's have a base of
     *  1-5
     */
    int casualties = g_Dice.in_range( 1, 6 );
    
    /*
     *  some more trait mods. Assasin adds an extra dice since
     *  she's been trained to deliver the coup-de-grace on a helpless
     *  opponent.
     */
    if( has_trait( "Assassin" ) )
    {
        casualties += g_Dice.in_range( 1, 6 );
    }
    
    if( has_trait( "Adventurer" ) ) // some level clearing instincts
    {
        casualties += 2;
    }
    
    if( has_trait( "Merciless" ) ) casualties ++;
    
    if( has_trait( "Yandere" ) ) casualties ++;
    
    if( has_trait( "Tsundere" ) ) casualties ++;
    
    if( has_trait( "Meek" ) ) casualties --;
    
    if( has_trait( "Dependent" ) ) casualties -= 2;
    
    /*
     *  fleet of foot means she gets out by running away more than fighting
     *  so fewer fatalities
     */
    if( has_trait( "Fleet of Foot" ) ) casualties -= 2;
    
    /*
     *  OK, apply the casualties and make sure it doesn't go negative
     */
    gang->m_Num -= casualties;
    
    if( gang->m_Num < 0 )
    {
        gang->m_Num = 0;
        
    }
}

void Girl::lose_vs_own_gang( std::vector<sGang*>& v, int max_goons, int girl_stats, int gang_stats, bool& girl_wins )
{
    sGang* gang = v[0];
    girl_wins = false;
    /*
     *  She's going to get hurt some. Moderating this, we have the fact that
     *  the goons don't really want to damage her (lest the boss get all shouty)
     *  However, depending on how good she is, they may not have a choice
     *
     *  also, I don't want to kill her if she was a full health. I hate it when
     *  that happens. You can send a girl to the dungeons and she's dead before
     *  you've even had a chance to twirl your moustaches at her.
     *  So we need to know how much health she had in the first place
     */
    int start_health = health();
    int damage = g_Dice.random( 12 ) * max_goons;
    
    /*
     *  and if the gangs had the advantage, reduce the
     *  damage actually taken accordingly
     */
    if( gang_stats > girl_stats )
    {
        damage *= girl_stats;
        damage /= gang_stats;
    }
    
    /*
     *  lastly, make sure this isn't going to kill her, if her health was
     *  > 90 in the first place
     */
    if( start_health >= 90 && damage >= start_health )
    {
        damage = start_health - 1;
    }
    
    health( -damage );
    /*
     *  need a bit more detail here, really...
     */
    int casualties = g_Dice.in_range( 1, 6 );
    casualties += 3;
    
    if( casualties < 0 ) casualties = 0;
    
    gang->m_Num -= casualties;
}

bool Girl::LoadGirlXML( TiXmlHandle hGirl )
{
    //this is always called after creating a new girl, so let's not init Girl again
    TiXmlElement* pGirl = hGirl.ToElement();
    
    if( pGirl == nullptr )
    {
        return false;
    }
    
    int tempInt = 0;
    
    // load the name
    const char* pTempString = pGirl->Attribute( "Name" );
    
    if( pTempString )
    {
        m_Name = new char[strlen( pTempString ) + 1];
        strcpy( m_Name, pTempString );
    }
    else
    {
        // HACK
        m_Name = const_cast<char*>( "" );
    }
    
    if( pGirl->Attribute( "Realname" ) )
    {
        m_Realname = pGirl->Attribute( "Realname" );
    }
    
    cConfig cfg;
    
    if( cfg.debug.log_girls() )
    {
        std::stringstream ss;
        ss << "Loading girl: '" << m_Realname;
        g_LogFile.write( ss.str() );
    }
    
    // get the description
    if( pGirl->Attribute( "Desc" ) )
    {
        m_Desc = pGirl->Attribute( "Desc" );
    }
    
    // load the amount of days they are unhappy in a row
    pGirl->QueryIntAttribute( "DaysUnhappy", &tempInt );
    m_DaysUnhappy = tempInt;
    tempInt = 0;
    
    // Load their traits
    LoadTraitsXML( hGirl.FirstChild( "Traits" ), m_NumTraits, m_Traits, m_TempTrait );
    
    if( m_NumTraits > MAXNUM_TRAITS )
        g_LogFile.write( "--- ERROR - Loaded more traits than girls can have??" );
        
    // Load their remembered traits
    LoadTraitsXML( hGirl.FirstChild( "Remembered_Traits" ), m_NumRememTraits, m_RememTraits );
    
    if( m_NumRememTraits > MAXNUM_TRAITS * 2 )
        g_LogFile.write( "--- ERROR - Loaded more remembered traits than girls can have??" );
        
    // Load inventory items
    LoadInventoryXML( hGirl.FirstChild( "Inventory" ), m_Inventory, m_NumInventory, m_EquipedItems );
    
    // load their states
    pGirl->QueryValueAttribute<long>( "States", &m_States );
    
    // load their stats
    LoadStatsXML( hGirl.FirstChild( "Stats" ), m_Stats, m_StatMods, m_TempStats );
    
    // load their skills
    LoadSkillsXML( hGirl.FirstChild( "Skills" ), m_Skills, m_SkillMods, m_TempSkills );
    
    // load virginity
    pGirl->QueryValueAttribute<bool>( "Virgin", &m_Virgin );
    
    // load using antipreg
    pGirl->QueryValueAttribute<bool>( "UseAntiPreg", &m_UseAntiPreg );
    
    // load withdrawals
    pGirl->QueryIntAttribute( "Withdrawals", &tempInt );
    m_Withdrawals = tempInt;
    tempInt = 0;
    
    // load money
    pGirl->QueryIntAttribute( "Money", &m_Money );
    
    // load acom level
    pGirl->QueryIntAttribute( "AccLevel", &tempInt );
    m_AccLevel = tempInt;
    tempInt = 0;
    // load day/night jobs
    pGirl->QueryIntAttribute( "DayJob", &tempInt );
    m_DayJob = tempInt;
    tempInt = 0;
    pGirl->QueryIntAttribute( "NightJob", &tempInt );
    m_NightJob = tempInt;
    tempInt = 0;
    
    // load prev day/night jobs
    pGirl->QueryIntAttribute( "PrevDayJob", &tempInt );
    m_PrevDayJob = tempInt;
    tempInt = 0;
    pGirl->QueryIntAttribute( "PrevNightJob", &tempInt );
    m_PrevNightJob = tempInt;
    tempInt = 0;
    
    // load runnayway value
    pGirl->QueryIntAttribute( "RunAway", &tempInt );
    m_RunAway = tempInt;
    tempInt = 0;
    
    // load spotted
    pGirl->QueryIntAttribute( "Spotted", &tempInt );
    m_Spotted = tempInt;
    tempInt = 0;
    
    // load weeks past, birth day, and pregant time
    pGirl->QueryValueAttribute<unsigned long>( "WeeksPast", &m_WeeksPast );
    pGirl->QueryValueAttribute<unsigned int>( "BDay", &m_BDay );
    pGirl->QueryIntAttribute( "WeeksPreg", &tempInt );
    m_WeeksPreg = tempInt;
    tempInt = 0;
    
    // load number of customers slept with
    pGirl->QueryValueAttribute<unsigned long>( "NumCusts", &m_NumCusts );
    
    // load girl flags
    TiXmlElement* pFlags = pGirl->FirstChildElement( "Flags" );
    
    if( pFlags )
    {
        std::string flagNumber;
        
        for( int i = 0; i < NUM_GIRLFLAGS; i++ )
        {
            flagNumber = "Flag_";
            std::stringstream stream;
            stream << i;
            flagNumber.append( stream.str() );
            pFlags->QueryIntAttribute( flagNumber, &tempInt );
            m_Flags[i] = tempInt;
            tempInt = 0;
        }
    }
    
    // load their torture value
    pGirl->QueryValueAttribute<bool>( "Tort", &m_Tort );
    
    // Load their children
    pGirl->QueryIntAttribute( "PregCooldown", &tempInt );
    m_PregCooldown = tempInt;
    tempInt = 0;
    
    
    // load number of children
    TiXmlElement* pChildren = pGirl->FirstChildElement( "Children" );
    
    if( pChildren )
    {
        for( TiXmlElement* pChild = pChildren->FirstChildElement( "Child" );
                pChild != nullptr;
                pChild = pChild->NextSiblingElement( "Child" ) )
        {
            sChild* child = new sChild();
            bool success = child->LoadChildXML( TiXmlHandle( pChild ) );
            
            if( success == true )
            {
                m_Children.add_child( child );
            }
            else
            {
                delete child;
                continue;
            }
        }
    }
    
    // load enjoyment values
    LoadActionsXML( hGirl.FirstChildElement( "Actions" ), m_Enjoyment );
    
    // load their triggers
    m_Triggers.LoadTriggersXML( hGirl.FirstChildElement( "Triggers" ) );
    m_Triggers.SetGirlTarget( this );
    
    if( m_Stats[STAT_AGE] < 17 )
        m_Stats[STAT_AGE] = 17;
        
    // load their images
    g_Girls.LoadGirlImages( this );
    g_Girls.CalculateGirlType( this );
    
    return true;
}

TiXmlElement* Girl::SaveGirlXML( TiXmlElement* pRoot )
{
    TiXmlElement* pGirl = new TiXmlElement( "Girl" );
    pRoot->LinkEndChild( pGirl );
    // save the name
    pGirl->SetAttribute( "Name", m_Name );
    
    // save the real name
    pGirl->SetAttribute( "Realname", m_Realname );
    
    // save the description
    pGirl->SetAttribute( "Desc", m_Desc );
    
    // save the amount of days they are unhappy
    pGirl->SetAttribute( "DaysUnhappy", m_DaysUnhappy );
    
    // Save their traits
    if( m_NumTraits > MAXNUM_TRAITS )
        g_LogFile.write( "---- ERROR - Saved more traits then girls can have" );
        
    SaveTraitsXML( pGirl, "Traits", MAXNUM_TRAITS, m_Traits, m_TempTrait );
    
    // Save their remembered traits
    if( m_NumRememTraits > MAXNUM_TRAITS * 2 )
        g_LogFile.write( "---- ERROR - Saved more remembered traits then girls can have" );
        
    SaveTraitsXML( pGirl, "Remembered_Traits", MAXNUM_TRAITS * 2, m_RememTraits, nullptr );
    
    // Save inventory items
    TiXmlElement* pInventory = new TiXmlElement( "Inventory" );
    pGirl->LinkEndChild( pInventory );
    SaveInventoryXML( pInventory, m_Inventory, 40, m_EquipedItems );
    
    // save their states
    pGirl->SetAttribute( "States", m_States );
    
    // Save their stats
    SaveStatsXML( pGirl, m_Stats, m_StatMods, m_TempStats );
    
    // save their skills
    SaveSkillsXML( pGirl, m_Skills, m_SkillMods, m_TempSkills );
    
    // save virginity
    pGirl->SetAttribute( "Virgin", m_Virgin );
    
    // save using antipreg
    pGirl->SetAttribute( "UseAntiPreg", m_UseAntiPreg );
    
    // save withdrawals
    pGirl->SetAttribute( "Withdrawals", m_Withdrawals );
    
    // save money
    pGirl->SetAttribute( "Money", m_Money );
    
    // save acom level
    pGirl->SetAttribute( "AccLevel", m_AccLevel );
    
    // save day/night jobs
    pGirl->SetAttribute( "DayJob", m_DayJob );
    pGirl->SetAttribute( "NightJob", m_NightJob );
    
    // save prev day/night jobs
    pGirl->SetAttribute( "PrevDayJob", m_PrevDayJob );
    pGirl->SetAttribute( "PrevNightJob", m_PrevNightJob );
    
    // save runnayway vale
    pGirl->SetAttribute( "RunAway", m_RunAway );
    
    // save spotted
    pGirl->SetAttribute( "Spotted", m_Spotted );
    
    // save weeks past, birth day, and pregant time
    pGirl->SetAttribute( "WeeksPast", m_WeeksPast );
    pGirl->SetAttribute( "BDay", m_BDay );
    pGirl->SetAttribute( "WeeksPreg", m_WeeksPreg );
    
    // number of customers slept with
    pGirl->SetAttribute( "NumCusts", m_NumCusts );
    
    // girl flags
    TiXmlElement* pFlags = new TiXmlElement( "Flags" );
    pGirl->LinkEndChild( pFlags );
    std::string flagNumber;
    
    for( int i = 0; i < NUM_GIRLFLAGS; i++ )
    {
        flagNumber = "Flag_";
        std::stringstream stream;
        stream << i;
        flagNumber.append( stream.str() );
        pFlags->SetAttribute( flagNumber, m_Flags[i] );
    }
    
    // save their torture value
    pGirl->SetAttribute( "Tort", m_Tort );
    
    // save their children
    pGirl->SetAttribute( "PregCooldown", m_PregCooldown );
    TiXmlElement* pChildren = new TiXmlElement( "Children" );
    pGirl->LinkEndChild( pChildren );
    sChild* child = m_Children.m_FirstChild;
    
    while( child )
    {
        child->SaveChildXML( pChildren );
        child = child->m_Next;
    }
    
    // save their enjoyment values
    SaveActionsXML( pGirl, m_Enjoyment );
    
    // save their triggers
    m_Triggers.SaveTriggersXML( pGirl );
    return pGirl;
}

void Girl::load_from_xml( TiXmlElement* el )
{
    int ival;
    const char* pt;
    
    /*
     *  get the simple fields
     */
    if( ( pt = el->Attribute( "Name" ) ) )
    {
        m_Name = n_strdup( pt );
        m_Realname = pt;
    }
    else
    {
        g_LogFile.ss() << "Error: can't find name when loading girl."
                       << "XML = " << ( *el ) << std::endl;
        g_LogFile.ssend();
        return;
    }
    
    if( ( pt = el->Attribute( "Desc" ) ) )
        m_Desc = n_strdup( pt );
        
    if( ( pt = el->Attribute( "Gold", &ival ) ) )
        m_Money = ival;
        
    /*
     *  loop through stats
     */
    for( int i = 0; i < NUM_STATS; i++ )
    {
        int ival;
        const char* stat_name = Girl::stat_names[i];
        pt = el->Attribute( stat_name, &ival );
        
        if( pt == nullptr )
        {
            g_LogFile.ss() << "Error: Can't find stat '" << stat_name << "' for girl '" << m_Realname << "'" << std::endl;
            g_LogFile.ssend();
            continue;
        }
        
        m_Stats[i] = ival;
        g_LogFile.ss() << "Debug: Girl='" << m_Realname << "'; Stat='" << stat_name << "'; Value='" << pt << "'; Ival = "
                       << int( m_Stats[i] ) << "'" << std::endl;
        g_LogFile.ssend();
    }
    
    // "fix" underage girls, determine virgin status
    m_Virgin = false;
    
    if( m_Stats[STAT_AGE] <= 17 )
    {
        m_Stats[STAT_AGE] = 17;
        m_Virgin = true;
    }
    else if( m_Stats[STAT_AGE] == 18 )
    {
        if( g_Dice % 3 == 1 )
            m_Virgin = true;
    }
    else if( m_Stats[STAT_AGE] <= 25 )
    {
        int avg = 0;
        
        for( u_int i = 0; i < NUM_SKILLS; i++ )
        {
            if(
                i != SKILL_SERVICE
                && i != SKILL_COMBAT
                && i != SKILL_MAGIC
            )
                avg += static_cast<int>( m_Skills[i] );
        }
        
        avg = avg / ( NUM_SKILLS - 1 );
        
        if( avg < 30 )
            m_Virgin = true;
    }
    
    /*
     *  loop through skills
     */
    for( u_int i = 0; i < NUM_SKILLS; i++ )
    {
        int ival;
        
        if( ( pt = el->Attribute( Girl::skill_names[i], &ival ) ) )
            m_Skills[i] = ival;
    }
    
    if( ( pt = el->Attribute( "Status" ) ) )
    {
        if( strcmp( pt, "Catacombs" ) == 0 )
            m_States |= ( 1 << STATUS_CATACOMBS );
        else if( strcmp( pt, "Slave" ) == 0 )
            m_States |= ( 1 << STATUS_SLAVE );
        else
            m_States = 0;
    }
    
    TiXmlElement* child;
    
    for( child = el->FirstChildElement(); child; child = child->NextSiblingElement() )
    {
        /*
         *      get the trait name
         */
        if( child->ValueStr() == "Trait" )
        {
            pt = child->Attribute( "Name" );
            m_Traits[m_NumTraits] = g_Traits.GetTrait( n_strdup( pt ) );
            m_NumTraits++;
        }
    }
    
    m_AccLevel = 1;
}

std::ostream& operator<<( std::ostream& os, Girl& g )
{
    os << g.m_Realname << std::endl;
    os << g.m_Desc << std::endl;
    os << std::endl;
    
    for( int i = 0; i < NUM_STATS; i++ )
    {
        os.width( 20 );
        os.flags( std::ios::left );
        os << g.stat_names[i]
           << "\t: "
           << int( g.m_Stats[i] )
           << std::endl
           ;
    }
    
    os << std::endl;
    
    for( u_int i = 0; i < NUM_SKILLS; i++ )
    {
        os.width( 20 );
        os.flags( std::ios::left );
        os << g.skill_names[i]
           << "\t: "
           << int( g.m_Skills[i] )
           << std::endl
           ;
    }
    
    os << std::endl;
    
    os << std::endl;
    return os;
}

cChildList::cChildList()
{
    ;
}

cChildList::~cChildList()
{
    if( m_FirstChild )
        delete m_FirstChild;
}

void cChildList::add_child( sChild* child )
{
    m_NumChildren++;
    
    if( m_LastChild )
    {
        m_LastChild->m_Next = child;
        child->m_Prev = m_LastChild;
        m_LastChild = child;
    }
    else
    {
        m_LastChild = child;
        m_FirstChild = child;
    }
    
}

sChild* cChildList::remove_child( sChild* child, Girl* girl )
{
    m_NumChildren--;
    sChild* temp = child->m_Next;
    
    if( child->m_Next )
        child->m_Next->m_Prev = child->m_Prev;
        
    if( child->m_Prev )
        child->m_Prev->m_Next = child->m_Next;
        
    if( child == girl->m_Children.m_FirstChild )
        girl->m_Children.m_FirstChild = child->m_Next;
        
    if( child == girl->m_Children.m_LastChild )
        girl->m_Children.m_LastChild = child->m_Prev;
        
    child->m_Next = nullptr;
    delete child;
    return temp;
}

} // namespace WhoreMasterRenewal
