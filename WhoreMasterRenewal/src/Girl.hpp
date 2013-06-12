#ifndef GIRL_HPP_INCLUDED_1616
#define GIRL_HPP_INCLUDED_1616

#include "Constants.h"
#include "cEvents.h"
#include "cTriggers.h"
#include "cGirls.h"
#include "cPlayer.h"

#include <string>
#include <vector>
#include <map>

class TiXmlElement;

namespace WhoreMasterRenewal
{

struct sTrait;
struct sInventoryItem;
class cAImgList;
struct sChild;
struct sGang;
class cPlayer;
struct sCustomer;



/*
a class to handle all the child related code to prevent errors.
*/
class cChildList
{
public:
    cChildList();
    ~cChildList();
    
    sChild* m_FirstChild;
    sChild* m_LastChild;
    int m_NumChildren;
    void add_child( sChild* );
    sChild* remove_child( sChild*, Girl* );
    //void handle_childs();
    //void save_data(std::ofstream);
    //void write_data(std::ofstream);
    //sChild * GenerateBornChild();//need to figure out what the player/customer base class is and if needed create one
    //sChild * GenerateUnbornChild();
    
};



// Represents a single girl
class Girl
{
public:
    char* m_Name;                               // The girls name
    std::string m_Realname;                         // this is the name displayed in text
    /*
     *  MOD: changed from char* -- easier to change from lua -- doc
     */
    std::string m_Desc;                             // Short story about the girl
    
    unsigned char m_NumTraits;                  // current number of traits they have
    sTrait* m_Traits[MAXNUM_TRAITS];            // List of traits they have
    unsigned char m_TempTrait[MAXNUM_TRAITS];   // a temp trait if not 0. Trait removed when == 0. traits last for 20 weeks.
    
    unsigned char m_NumRememTraits;             // number of traits that are apart of the girls starting traits
    sTrait* m_RememTraits[MAXNUM_TRAITS * 2];   // List of traits they have inbuilt
    
    unsigned char m_DayJob;                     // id for what job the girl is currently doing
    unsigned char m_NightJob;                   // id for what job the girl is currently doing
    unsigned char m_PrevDayJob;                 // id for what job the girl was doing
    unsigned char m_PrevNightJob;               // id for what job the girl was doing
    
    //ADB needs to be int because player might have more than 256
    int m_NumInventory;                         // current amount of inventory they have
    sInventoryItem* m_Inventory[40];            // List of inventory items they have (40 max)
    unsigned char m_EquipedItems[40];           // value of > 0 means equipped (wearing) the item
    
    long m_States;                              // Holds the states the girl has
    
    // Abstract stats (not shown as numbers but as a raiting)
    unsigned char m_Stats[NUM_STATS];
    int m_StatMods[NUM_STATS];                  // perminant modifiers to stats
    int m_TempStats[NUM_STATS];                 // these go down (or up) by 30% each week until they reach 0
    
    int m_Enjoyment[NUM_ACTIONTYPES];           // these values determine how much a girl likes an action
    // (-100 is hate, +100 is loves)
    bool m_Virgin;                              // is she a virgin
    
    bool m_UseAntiPreg;                         // if true she will use anit preg measures
    
    unsigned char m_Withdrawals;                // if she is addicted to something this counts how many weeks she has been off
    
    int m_Money;
    
    unsigned char m_AccLevel;                   // how good her accomadation is, 0 is slave like and non-slaves will really hate it
    
    unsigned char m_Skills[NUM_SKILLS];
    int m_SkillMods[NUM_SKILLS];
    int m_TempSkills[NUM_SKILLS];               // these go down (or up) by 1 each week until they reach 0
    
    unsigned char m_RunAway;                    // if 0 then off, if 1 then girl is removed from list,
    // otherwise will count down each week
    unsigned char m_Spotted;                    // if 1 then she has been seen stealing but not punished yet
    
    unsigned long m_WeeksPast;                  // number of weeks in your service
    unsigned int m_BDay;                        // number of weeks in your service since last aging
    
    unsigned long m_NumCusts;                   // number of customers this girl has slept with
    
    bool m_Tort;                                // if true then have already tortured today
    bool m_JustGaveBirth;                       // did she give birth this current week?
    
    int m_Pay;                                  // used to keep track of pay this turn
    
    cAImgList* m_GirlImages;                    // Images to display
    
    long m_FetishTypes;                         // the types of fetishes this girl has
    
    unsigned char m_Flags[NUM_GIRLFLAGS];       // flags used by scripts
    
    cEvents m_Events;                           // Each girl keeps track of all her events that happened to her in the last turn
    
    
    cTriggerList m_Triggers;                    // triggers for the girl
    
    unsigned char m_DaysUnhappy;                // used to track how many days they are really unhappy for
    
    Girl* m_Next;
    Girl* m_Prev;
    
    unsigned char m_WeeksPreg;                  // number of weeks pregnant or inseminated
    unsigned char m_PregCooldown;               // number of weeks until can get pregnant again
    cChildList m_Children;
    
    Girl();
    ~Girl();
    
    void dump( std::ostream& os );
    
    /*
     *  MOD: docclox. attach the skill and stat names to the
     *  class that uses them. Plus an XML load method and
     *  an ostream << operator to pretty print the struct for
     *  debug purposes.
     *
     *  Sun Nov 15 05:58:55 GMT 2009
     */
    static const char* stat_names[];
    static const char* skill_names[];
    static const char* status_names[];
    /*
     *  again, might as well make them part of the struct that uses them
     */
    static const unsigned int max_stats;
    static const unsigned int max_skills;
    static const unsigned int max_statuses;
    /*
     *  we need to be able to go the other way, too:
     *  from string to number. The maps map stat/skill names
     *  onto index numbers. The setup flag is so we can initialise
     *  the maps the first time an Girl is constructed
     */
    static bool m_maps_setup;
    static std::map<std::string, unsigned int> stat_lookup;
    static std::map<std::string, unsigned int> skill_lookup;
    static std::map<std::string, unsigned int> status_lookup;
    static void setup_maps();
    
    static int lookup_stat_code( std::string s );
    static int lookup_skill_code( std::string s );
    static int lookup_status_code( std::string s );
    /*
     *  Strictly speaking, methods don't belong in structs.
     *  I've always thought that more of a guideline than a hard and fast rule
     */
    void load_from_xml( TiXmlElement* el );
    TiXmlElement* SaveGirlXML( TiXmlElement* pRoot );
    bool LoadGirlXML( TiXmlHandle hGirl );
    
    /*
     *  stream operator - used for debug
     */
    friend std::ostream& operator<<( std::ostream& os, Girl& g );
    /*
     *  it's a bit daft that we have to go through the global g_Girls
     *  every time we want a stat.
     *
     *  I mean the Girl type is the one we're primarily concerned with.
     *  that ought to be the base for the query.
     *
     *  Of course, I could just index into the stat array,
     *  but I'm not sure what else the cGirls method does.
     *  So this is safer, if a bit inefficient.
     */
    bool calc_pregnancy( int, cPlayer* );
    int get_stat( int stat_id );
    int upd_temp_stat( int stat_id, int amount );
    int upd_stat( int stat_id, int amount );
    /*
     *  Now then:
     */
    int charisma()
    {
        return get_stat( STAT_CHARISMA );
    }
    int charisma( int n )
    {
        return upd_stat( STAT_CHARISMA, n );
    }
    int happiness()
    {
        return get_stat( STAT_HAPPINESS );
    }
    int happiness( int n )
    {
        return upd_stat( STAT_HAPPINESS, n );
    }
    int libido()
    {
        return get_stat( STAT_LIBIDO );
    }
    int libido( int n )
    {
        return upd_stat( STAT_LIBIDO, n );
    }
    int constitution()
    {
        return get_stat( STAT_CONSTITUTION );
    }
    int constitution( int n )
    {
        return upd_stat( STAT_CONSTITUTION, n );
    }
    int intelligence()
    {
        return get_stat( STAT_INTELLIGENCE );
    }
    int intelligence( int n )
    {
        return upd_stat( STAT_INTELLIGENCE, n );
    }
    int confidence()
    {
        return get_stat( STAT_CONFIDENCE );
    }
    int confidence( int n )
    {
        return upd_stat( STAT_CONFIDENCE, n );
    }
    int mana()
    {
        return get_stat( STAT_MANA );
    }
    int mana( int n )
    {
        return upd_stat( STAT_MANA, n );
    }
    int agility()
    {
        return get_stat( STAT_AGILITY );
    }
    int agility( int n )
    {
        return upd_stat( STAT_AGILITY, n );
    }
    int fame()
    {
        return get_stat( STAT_FAME );
    }
    int fame( int n )
    {
        return upd_stat( STAT_FAME, n );
    }
    int level()
    {
        return get_stat( STAT_LEVEL );
    }
    int level( int n )
    {
        return upd_stat( STAT_LEVEL, n );
    }
    int askprice()
    {
        return get_stat( STAT_ASKPRICE );
    }
    int askprice( int n )
    {
        return upd_stat( STAT_ASKPRICE, n );
    }
    /* It's NOT lupus! */
    int house()
    {
        return get_stat( STAT_HOUSE );
    }
    int house( int n )
    {
        return upd_stat( STAT_HOUSE, n );
    }
    int exp()
    {
        return get_stat( STAT_EXP );
    }
    int exp( int n )
    {
        return upd_stat( STAT_EXP, n );
    }
    int age()
    {
        return get_stat( STAT_AGE );
    }
    int age( int n )
    {
        return upd_stat( STAT_AGE, n );
    }
    int obedience()
    {
        return get_stat( STAT_OBEDIENCE );
    }
    int obedience( int n )
    {
        return upd_stat( STAT_OBEDIENCE, n );
    }
    int spirit()
    {
        return get_stat( STAT_SPIRIT );
    }
    int spirit( int n )
    {
        return upd_stat( STAT_SPIRIT, n );
    }
    int beauty()
    {
        return get_stat( STAT_BEAUTY );
    }
    int beauty( int n )
    {
        return upd_stat( STAT_BEAUTY, n );
    }
    int tiredness()
    {
        return get_stat( STAT_TIREDNESS );
    }
    int tiredness( int n )
    {
        return upd_stat( STAT_TIREDNESS, n );
    }
    int health()
    {
        return get_stat( STAT_HEALTH );
    }
    int health( int n )
    {
        return upd_stat( STAT_HEALTH, n );
    }
    int pcfear()
    {
        return get_stat( STAT_PCFEAR );
    }
    int pcfear( int n )
    {
        return upd_stat( STAT_PCFEAR, n );
    }
    int pclove()
    {
        return get_stat( STAT_PCLOVE );
    }
    int pclove( int n )
    {
        return upd_stat( STAT_PCLOVE, n );
    }
    int pchate()
    {
        return get_stat( STAT_PCHATE );
    }
    int pchate( int n )
    {
        return upd_stat( STAT_PCHATE, n );
    }
    /*
     *  notice that if we do tweak get_stat to reference the stats array
     *  direct, the above still work.
     *
     *  similarly...
     */
    int get_skill( int skill_id );
    int upd_temp_skill( int skill_id, int amount );
    int upd_skill( int skill_id, int amount );
    int anal()
    {
        return get_skill( SKILL_ANAL );
    }
    int anal( int n )
    {
        return upd_skill( SKILL_ANAL, n );
    }
    int bdsm()
    {
        return get_skill( SKILL_BDSM );
    }
    int bdsm( int n )
    {
        return upd_skill( SKILL_BDSM, n );
    }
    int beastiality()
    {
        return get_skill( SKILL_BEASTIALITY );
    }
    int beastiality( int n )
    {
        return upd_skill( SKILL_BEASTIALITY, n );
    }
    int combat()
    {
        return get_skill( SKILL_COMBAT );
    }
    int combat( int n )
    {
        return upd_skill( SKILL_COMBAT, n );
    }
    int group()
    {
        return get_skill( SKILL_GROUP );
    }
    int group( int n )
    {
        return upd_skill( SKILL_GROUP, n );
    }
    int lesbian()
    {
        return get_skill( SKILL_LESBIAN );
    }
    int lesbian( int n )
    {
        return upd_skill( SKILL_LESBIAN, n );
    }
    int magic()
    {
        return get_skill( SKILL_MAGIC );
    }
    int magic( int n )
    {
        return upd_skill( SKILL_MAGIC, n );
    }
    int normalsex()
    {
        return get_skill( SKILL_NORMALSEX );
    }
    int normalsex( int n )
    {
        return upd_skill( SKILL_NORMALSEX, n );
    }
    int service()
    {
        return get_skill( SKILL_SERVICE );
    }
    int service( int n )
    {
        return upd_skill( SKILL_SERVICE, n );
    }
    int strip()
    {
        return get_skill( SKILL_STRIP );
    }
    int strip( int n )
    {
        return upd_skill( SKILL_STRIP, n );
    }
    
    
    /*
     *  convenience func. Also easier to read like this
     */
    bool carrying_monster();
    bool carrying_human();
    bool carrying_players_child();
    bool carrying_customer_child();
    bool is_pregnant();
    bool is_mother();
    bool is_poisoned();
    void clear_pregnancy();
    int preg_chance( int base_pc, bool good = false, double factor = 1.0 );
    bool calc_pregnancy( cPlayer* player, bool good = false, double factor = 1.0 );
    bool calc_pregnancy( sCustomer* cust, bool good = false, double factor = 1.0 );
    bool calc_insemination( sCustomer* cust, bool good = false, double factor = 1.0 );
    bool calc_insemination( cPlayer* player, bool good = false, double factor = 1.0 );
    /*
     *  let's overload that...
     *  should be able to do the same using sCustomer as well...
     */
    void add_trait( std::string trait, bool temp = true );
    bool has_trait( std::string trait );
    bool is_addict();
    sChild* next_child( sChild* child, bool remove = false );
    int preg_type( int image_type );
    Girl* run_away();
    
    bool is_slave();
    bool is_free();
    void set_slave();
    bool is_monster();
    bool is_human();
    
    void fight_own_gang( bool& girl_wins );
    void win_vs_own_gang( std::vector<sGang*>& v, int max_goons, bool& girl_wins );
    void lose_vs_own_gang(
        std::vector<sGang*>& v,
        int max_goons,
        int girl_stats,
        int gang_stats,
        bool& girl_wins
    );
    
    void OutputGirlRow( std::string* Data, const std::vector<std::string>& columnNames );
    void OutputGirlDetailString( std::string& Data, const std::string& detailName );
    
    // END MOD
};

} // namespace

#endif // GIRL_HPP_INCLUDED_1616
