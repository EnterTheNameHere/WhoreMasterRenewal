#ifndef GANGMANAGER_HPP_INCLUDED_2044
#define GANGMANAGER_HPP_INCLUDED_2044

#include <vector>
#include <fstream>

class TiXmlElement;
class TiXmlHandle;

namespace WhoreMasterRenewal
{

class cGangManager;
extern cGangManager g_Gangs;

class sGang;
class Girl;
typedef unsigned int u_int;

/*
 * manages all the player gangs
 */
class cGangManager
{
public:
    cGangManager();
    ~cGangManager();
    
    cGangManager( const cGangManager& ) = delete;
    cGangManager& operator = ( const cGangManager& ) = delete;
    
    void Free();
    
    void AddNewGang( bool boosted = false ); // Adds a new randomly generated gang to the recruitable list
    void HireGang( int gangID ); // hired a recruitable gang, so add it to your gangs
    void FireGang( int gangID ); // fired a gang, so send it back to recruitables (or just delete if full up)
    void AddHireableGang( sGang* gang );
    void AddGang( sGang* gang );
    void RemoveHireableGang( int gangID );  // removed a recruitable gang from the list
    void RemoveHireableGang( sGang* gang );
    void RemoveGang( int gangID );  // removed a controlled gang completely from service
    void RemoveGang( sGang* gang );
    void SendGang( int gangID, int missID ); // sends a gang on a mission
    sGang* GetGang( int gangID ); // gets a gang
    sGang* GetHireableGang( int gangID ); // gets a recruitable gang
    sGang* GetGangOnMission( u_int missID ); // gets a gang on the current mission
    void UpdateGangs();
    int GetNumGangs();
    int GetNumHireableGangs();
    bool GangCombat( Girl* girl, sGang* gang ); // returns true if the girl wins
    bool GangBrawl( sGang* gang1, sGang* gang2 ); // returns true if gang1 wins
    sGang* GetTempGang();   // creates a new gang
    void BoostGangSkill( unsigned char* affect_skill, int count = 1 ); // increases a specific skill/stat the specified number of times
    void BoostGangRandomSkill( std::vector<unsigned char*>* possible_skills, int count = 1, int boost_count = 1 ); // chooses from the passed skills/stats and raises one or more of them
    void BoostGangCombatSkills( sGang* gang, int count = 1 ); // simple function to increase a gang's combat skills a bit
    
    sGang* GetTempWeakGang();
    
    TiXmlElement* SaveGangsXML( TiXmlElement* pRoot );
    bool LoadGangsXML( TiXmlHandle hGangManager );
    void LoadGangsLegacy( std::ifstream& ifs );
    
    int GetNumBusinessExtorted() {return m_BusinessesExtort;}
    
    int* GetWeaponLevel() {return &m_SwordLevel;}
    int* GetNets() {return &m_NumNets;}
    int GetNetRestock() {return m_KeepNetsStocked;}
    void KeepNetStocked( int stocked ) {m_KeepNetsStocked = stocked;}
    int* GetHealingPotions() {return &m_NumHealingPotions;}
    void KeepHealStocked( int stocked ) {m_KeepHealStocked = stocked;}
    int GetHealingRestock() {return m_KeepHealStocked;}
    
    void sabotage_mission( sGang* gang );
    bool recapture_mission( sGang* gang );
    int chance_to_catch( Girl* girl );
    int healing_limit();
    
    // Used by the new brothel security code
    sGang* random_gang( std::vector<sGang*>& v );
    bool GirlVsEnemyGang( Girl* girl, sGang* enemy_gang );
    
    std::vector<sGang*> gangs_on_mission( u_int mission_id );
    
private:
    int m_BusinessesExtort = 0; // number of businesses under your control
    
    int m_NumGangNames = 0;
    unsigned char m_NumGangs = 0;
    sGang* m_GangStart = nullptr; // the start and end of the list of gangs under the players employment
    sGang* m_GangEnd = nullptr;
    unsigned char m_NumHireableGangs = 0;
    sGang* m_HireableGangStart = nullptr; // the start and end of the list of gangs which are available for hire
    sGang* m_HireableGangEnd = nullptr;
    
    // gang armory
    // mod - changing the keep stocked flag to an int
    // so we can record the level at which to maintain
    // the stock - then we can restock at turn end
    // to prevent squads becoming immortal by
    // burning money
    int m_KeepHealStocked = 0;
    int m_NumHealingPotions = 0;
    int m_SwordLevel = 0;
    int m_KeepNetsStocked = 0;
    int m_NumNets = 0;
};

} // namespace WhoreMasterRenewal

#endif // GANGMANAGER_HPP_INCLUDED_2044
