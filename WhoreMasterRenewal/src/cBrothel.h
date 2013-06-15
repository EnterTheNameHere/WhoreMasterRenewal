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
#ifndef CBROTHEL_H_INCLUDED_1533
#define CBROTHEL_H_INCLUDED_1533
#pragma once

#include "cRival.h" // required cRivalManager
#include "cGold.h" // required cGold
#include "cJobManager.h" // required cJobManager
#include "cBuilding.h" // required cBuilding
#include "cDungeon.h" // required cDungeon
#include "cPlayer.h" // required cPlayer
#include "cEvents.h" // required cEvents

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

class TiXmlElement;
class TiXmlHandle;

namespace WhoreMasterRenewal
{

class cBrothelManager;
extern cBrothelManager g_Brothels;
extern int g_CurrBrothel;

class cBuilding;
class cGirls;
class cCustomers;
class cEvents;
struct sInventoryItem;
class Girl;

// holds an objective and its data
typedef struct sObjective
{
    u_int m_Objective;  // the objective type
    int m_Reward;       // the reward type
    int m_Target;       // the x variable for the objective
    int m_SoFar;        // how much of the x variable has been achieved
    int m_Limit;        // the number of weeks must be done by
    int m_Difficulty;   // a number representing how hard it is
} sObjective;



// defines a single brothel
class Brothel
{
public:
    Brothel();                         // constructor
    ~Brothel();                        // destructor
    
    Brothel( const Brothel& ) = delete;
    Brothel& operator = ( const Brothel& ) = delete;
    
    int m_id;
    std::string m_Name;
    unsigned short m_Happiness;        // av. % happy customers last week
    unsigned short m_TotalCustomers;   // the total number of customers for the last week
    unsigned short m_MiscCustomers;    // customers used for temp purposes but must still be taken into account
    unsigned char m_Fame;             // How famous this brothel is
    unsigned char m_NumRooms;         // How many rooms it has
    unsigned char m_Bar;              // level of bar: 0 => none
    unsigned char m_GamblingHall;     // as above
    unsigned char m_NumGirls;         // How many girls are here
    unsigned short m_AdvertisingBudget;// Budget player has set for weekly advertising
    double m_AdvertisingLevel; // multiplier for how far budget goes, based on girls working in advertising
    
    cBuilding m_Building;
    
    // for keeping track of how well the place is doing (for the last week)
    cGold m_Finance;
    
    unsigned char m_HasGambStaff;     // gambling hall or
    unsigned char m_HasBarStaff;      // Bar staff. Not as good as girls but consistent
    
    // non private variables (just cause it makes life so much easier)
    bool m_RestrictAnal;
    bool m_RestrictBDSM;
    bool m_RestrictBeast;
    bool m_RestrictGroup;
    bool m_RestrictNormal;
    bool m_RestrictLesbian;
    
    int m_Filthiness;
    
    cEvents m_Events;
    
    int m_BuildingQuality[NUMJOBTYPES];
    
    Girl* m_Girls;                // A list of all the girls this place has
    Girl* m_LastGirl;
    Brothel* m_Next;
    
    int m_SecurityLevel;
    
    // mod docclox - convenience funcs
    int free_rooms();
    bool matron_on_shift( int shift );
    bool has_matron();
    int matron_count();
    // end mod
    TiXmlElement* SaveBrothelXML( TiXmlElement* pRoot );
    bool LoadBrothelXML( TiXmlHandle hBrothel );
    void AddGirl( Girl* pGirl );
    
};


/*
 * manages all the player brothels
 *
 * Anyone else think this class tries to do too much?
 * Yes it does, I am working on reducing it-Delta
 */
class cBrothelManager
{
public:
    cBrothelManager();                  // constructor
    ~cBrothelManager();                 // destructor
    
    cBrothelManager( const cBrothelManager& ) = delete;
    cBrothelManager& operator = ( const cBrothelManager& ) = delete;
    
    void Free();
    
    Girl* GetDrugPossessor();
    
    void AddGirlToPrison( Girl* girl );
    void RemoveGirlFromPrison( Girl* girl );
    int GetNumInPrison()
    {
        return m_NumPrison;
    }
    
    void AddGirlToRunaways( Girl* girl );
    void RemoveGirlFromRunaways( Girl* girl );
    int GetNumRunaways()
    {
        return m_NumRunaways;
    }
    
    void NewBrothel( int NumRooms );
    void DestroyBrothel( int ID );
    void UpdateBrothels();
    void UpdateGirls( Brothel* brothel, int DayNight );
    // MYR: Start of my automation functions
    void UsePlayersItems( Girl* cur );
    bool AutomaticItemUse( Girl* girl, int InvNum, std::string message );
    bool AutomaticSlotlessItemUse( Girl* girl, int InvNum, std::string message );
    bool AutomaticFoodItemUse( Girl* girl, int InvNum, std::string message );
    bool RemoveItemFromInventoryByNumber( int Pos ); // support fn
    // End of automation functions
    void UpdateAllGirlsStat( Brothel* brothel, int stat, int amount );
    void SetGirlStat( Girl* girl, int stat, int amount );
    
    Girl* GetPrison()
    {
        return m_Prison;
    }
    //mod needed for convenience
    int& stat_lookup( std::string stat_name, int brothel_id = -1 );
    // jobs moving to their own class
    
    int GetGirlsCurrentBrothel( Girl* girl ); // Used by new security guard code
    std::vector<Girl*> GirlsOnJob( int BrothelID, int JobID, bool day ); // Also used by new security code
    
    bool UseAntiPreg( bool use );
    void AddAntiPreg( int amount );
    int GetNumPotions()
    {
        return m_AntiPregPotions;
    }
    void KeepPotionsStocked( bool stocked )
    {
        m_KeepPotionsStocked = stocked;
    }
    bool GetPotionRestock()
    {
        return m_KeepPotionsStocked;
    }
    
    int GetTotalNumGirls( bool monster = false );
    
    void UpgradeSupplySheds()
    {
        m_SupplyShedLevel++ ;
    }
    int  GetSupplyShedLevel()
    {
        return m_SupplyShedLevel;
    }
    
    void AddGirl( int brothelID, Girl* girl );
    void RemoveGirl( int brothelID, Girl* girl, bool deleteGirl = true );
    Girl* GetFirstRunaway();
    void sort( Brothel* brothel );      // sorts the list of girls
    void SortInventory();
    
    void SetName( int brothelID, std::string name );
    std::string GetName( int brothelID );
    
    bool CheckBarStaff( Brothel* brothel, int numGirls );  // returns true if the bar is staffed
    bool CheckGambStaff( Brothel* brothel, int numGirls ); // as above but for gambling hall
    
    bool FightsBack( Girl* girl );
    int GetNumGirls( int brothelID );
    std::string GetGirlString( int brothelID, int girlNum );
    int GetNumGirlsOnJob( int brothelID, int jobID, bool day );
    
    std::string GetBrothelString( int brothelID );
    
    Girl* GetGirl( int brothelID, int num );
    int GetGirlPos( int brothelID, Girl* girl );
    Girl* GetGirlByName( int brothelID, std::string name ); // MYR: Used by new end of turn code in InerfaceProcesses::TurnSummary
    
    Brothel* GetBrothel( int brothelID );
    int GetNumBrothels()
    {
        return m_NumBrothels;
    }
    int GetNumBrothelsWithVacancies();
    
    void CalculatePay( Brothel* brothel, Girl* girl, u_int Job );
    
    bool PlayerCombat( Girl* girl ); // returns true if the girl wins
    
    cPlayer* GetPlayer()
    {
        return &m_Player;
    }
    cDungeon* GetDungeon()
    {
        return &m_Dungeon;
    }
    
    int HasItem( std::string name, int countFrom = -1 );
    
    // Some public members for ease of use
    //ADB needs to be int because player might have more than 256, look, MAXNUM_INVENTORY == 300!
    int m_NumInventory;                             // current amount of inventory the brothel has
    sInventoryItem* m_Inventory[MAXNUM_INVENTORY];  // List of inventory items they have (300 max)
    unsigned char m_EquipedItems[MAXNUM_INVENTORY]; // value of > 0 means equipped (wearing) the item
    unsigned char m_NumItem[MAXNUM_INVENTORY];      // the number of items there are stacked
    cJobManager m_JobManager;                       // manages all the jobs
    
    long GetBribeRate()
    {
        return m_BribeRate;
    }
    void SetBribeRate( long rate )
    {
        m_BribeRate = rate;
    }
    void UpdateBribeInfluence();
    int  GetInfluence()
    {
        return m_Influence;
    }
    
    cRival* GetRivals()
    {
        return m_Rivals.GetRivals();
    }
    cRivalManager* GetRivalManager()
    {
        return &m_Rivals;
    }
    
    void WithdrawFromBank( long amount );
    void DepositInBank( long amount );
    long GetBankMoney()
    {
        return m_Bank;
    }
    void add_to_beasts( int i )
    {
        m_Beasts += i;
        
        if( m_Beasts < 0 )
            m_Beasts = 0;
    }
    int GetNumBeasts()
    {
        return m_Beasts;
    }
    bool CheckScripts();
    
    void UpdateObjective();             // updates an objective and checks for compleation
    sObjective* GetObjective();         // returns the objective
    void CreateNewObjective();          // Creates a new objective
    void PassObjective();               // Gives a reward
    void AddCustomObjective( int limit, int diff, int objective, int reward, int sofar, int target );
    
    TiXmlElement* SaveDataXML( TiXmlElement* pRoot );
    bool LoadDataXML( TiXmlHandle hBrothelManager );
    void LoadDataLegacy( std::ifstream& ifs );
    
    bool NameExists( std::string name );
    
    bool AddItemToInventory( sInventoryItem* item );
    
    void check_druggy_girl( std::stringstream& ss );
    void check_raid();
    void do_tax();
    void check_rivals();
    std::string new_rival_text();
    void do_food_and_digs( Brothel* brothel, Girl* girl );
    std::string disposition_text();
    std::string fame_text( Brothel* brothel );
    std::string suss_text();
    std::string happiness_text( Brothel* brothel );
    double calc_pilfering( Girl* girl );
    void peace_breaks_out();
    
    bool runaway_check( Brothel* brothel, Girl* girl );
    
    
    // WD: JOB_TORTURER stuff
    void TortureDone( bool flag )
    {
        m_TortureDoneFlag = flag;
        return;
    }
    bool TortureDone()
    {
        return m_TortureDoneFlag;
    }
    Girl* WhoHasTorturerJob();
    
    // WD: test to check if doing turn processing.  Used to ingnore HOUSE_STAT value in GetRebelValue() if girl gets to keep all her income.
    bool is_Dayshift_Processing()
    {
        return m_Processing_Shift == SHIFT_DAY;
    }
    bool is_Nightshift_Processing()
    {
        return m_Processing_Shift == SHIFT_NIGHT;
    }
    
private:
    int TotalFame( Brothel* );
    cPlayer m_Player;               // the stats for the player owning these brothels
    cDungeon m_Dungeon;             // the dungeon
    
    int m_NumBrothels;
    Brothel* m_Parent;
    Brothel* m_Last;
    
    // brothel supplies
    bool m_KeepPotionsStocked;
    int m_AntiPregPotions;         // the number of pregnancy/insimination preventive potions in stock
    int m_SupplyShedLevel;         // the level of the supply sheds. the higher the level, the more alcohol and antipreg potions can hold
    
    // brothel resources
    int m_HandmadeGoods;            // used with the community centre
    int m_Beasts;                   // used for beastiality scenes
    int m_AlchemyIngredients;
    
    int m_NumPrison;
    Girl* m_Prison;                // a list of girls kept in prision
    Girl* m_LastPrison;
    
    int m_NumRunaways;
    Girl* m_Runaways;
    Girl* m_LastRunaway;
    
    long m_BribeRate;               // the amount of money spent bribing officials per week
    int m_Influence;               // based on the bribe rate this is the percentage of influence you have
    int m_Dummy;                   //a dummy variable
    long m_Bank;                    // how much is stored in the bank
    
    sObjective* m_Objective;
    
    cRivalManager m_Rivals;         // all of the players compedators
    
    bool m_TortureDoneFlag;         // WD:  Have we got a torturer working today
    int m_Processing_Shift;        // WD:  Store DayNight value when processing girls
    
    // WD:  Update code of girls stats
    void updateGirlTurnBrothelStats( Girl* girl );
    
    void AddBrothel( Brothel* newBroth );
};

} // namespace WhoreMasterRenewal

#endif  // CBROTHEL_H_INCLUDED_1533
