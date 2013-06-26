#ifndef BROTHELMANAGER_HPP_INCLUDED_1636
#define BROTHELMANAGER_HPP_INCLUDED_1636

#include "cPlayer.h" // required cPlayer
#include "cDungeon.h" // required cDungeon
#include "cJobManager.h" // required cJobManager
#include "cRival.h" // required cRivalManager

#include <string>
#include <vector>

class TiXmlElement;

namespace WhoreMasterRenewal
{

class BrothelManager;
extern BrothelManager g_Brothels;
extern int g_CurrBrothel;

typedef unsigned int u_int;

class Girl;
class Brothel;
class sInventoryItem;



Girl* girl_sort( Girl* girl, Girl** lastgirl );
int sGirlcmp( Girl* a, Girl* b );



// holds an objective and its data
struct Objective
{
    u_int m_Objective = 0;  // the objective type
    int m_Reward = 0;       // the reward type
    int m_Target = 0;       // the x variable for the objective
    int m_SoFar = 0;        // how much of the x variable has been achieved
    int m_Limit = 0;        // the number of weeks must be done by
    int m_Difficulty = 0;   // a number representing how hard it is
};

/*
 * manages all the player brothels
 *
 * Anyone else think this class tries to do too much?
 * Yes it does, I am working on reducing it-Delta
 */
class BrothelManager
{
public:
    BrothelManager();
    ~BrothelManager();
    
    BrothelManager( const BrothelManager& ) = delete;
    BrothelManager& operator = ( const BrothelManager& ) = delete;
    
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
    // NOTE UNUSED void DestroyBrothel( int ID );
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
    // NOTE UNUSED int& stat_lookup( std::string stat_name, int brothel_id = -1 );
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
    
    /*
    // NOTE UNUSED void UpgradeSupplySheds()
    {
        m_SupplyShedLevel++ ;
    }
    */
    
    int GetSupplyShedLevel()
    {
        return m_SupplyShedLevel;
    }
    
    void AddGirl( int brothelID, Girl* girl );
    void RemoveGirl( int brothelID, Girl* girl, bool deleteGirl = true );
    Girl* GetFirstRunaway();
    // NOTE UNUSED void sort( Brothel* brothel );      // sorts the list of girls
    void SortInventory();
    
    void SetName( int brothelID, std::string name );
    std::string GetName( int brothelID );
    
    // NOTE UNUSED bool CheckBarStaff( Brothel* brothel, int numGirls );  // returns true if the bar is staffed
    // NOTE UNUSED bool CheckGambStaff( Brothel* brothel, int numGirls ); // as above but for gambling hall
    
    bool FightsBack( Girl* girl );
    int GetNumGirls( int brothelID );
    // NOTE UNUSED std::string GetGirlString( int brothelID, int girlNum );
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
    
    // NOTE UNUSED int GetNumBrothelsWithVacancies();
    
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
    
    long GetBribeRate()
    {
        return m_BribeRate;
    }
    
    void SetBribeRate( long rate )
    {
        m_BribeRate = rate;
    }
    
    void UpdateBribeInfluence();
    int GetInfluence()
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
    
    void UpdateObjective();     // updates an objective and checks for compleation
    Objective* GetObjective();  // returns the objective
    void CreateNewObjective();  // Creates a new objective
    void PassObjective();       // Gives a reward
    // NOTE UNUSED void AddCustomObjective( int limit, int diff, int objective, int reward, int sofar, int target );
    
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
    
    // WD: test to check if doing turn processing. Used to ingnore HOUSE_STAT value in GetRebelValue() if girl gets to keep all her income.
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
    void AddBrothel( Brothel* newBroth );
    
    // WD: Update code of girls stats
    void updateGirlTurnBrothelStats( Girl* girl );
    
    
    
public:
    // Some public members for ease of use
    //ADB needs to be int because player might have more than 256, look, MAXNUM_INVENTORY == 300!
    int m_NumInventory = 0;                         // current amount of inventory the brothel has
    sInventoryItem* m_Inventory[MAXNUM_INVENTORY];  // List of inventory items they have (300 max)
    unsigned char m_EquipedItems[MAXNUM_INVENTORY]; // value of > 0 means equipped (wearing) the item
    unsigned char m_NumItem[MAXNUM_INVENTORY];      // the number of items there are stacked
    cJobManager m_JobManager;                       // manages all the jobs

private:
    cPlayer m_Player = {};              // the stats for the player owning these brothels
    cDungeon m_Dungeon = {};            // the dungeon
    
    int m_NumBrothels = 0;
    Brothel* m_Parent = nullptr;
    Brothel* m_Last = nullptr;
    
    // brothel supplies
    bool m_KeepPotionsStocked = false;
    int m_AntiPregPotions = 0;          // the number of pregnancy/insimination preventive potions in stock
    int m_SupplyShedLevel = 1;          // the level of the supply sheds. the higher the level, the more alcohol and antipreg potions can hold
    
    // brothel resources
    int m_HandmadeGoods = 0;            // used with the community centre
    int m_Beasts = 0;                   // used for beastiality scenes
    int m_AlchemyIngredients = 0;
    
    int m_NumPrison = 0;
    Girl* m_Prison = nullptr;           // a list of girls kept in prision
    Girl* m_LastPrison = nullptr;
    
    int m_NumRunaways = 0;
    Girl* m_Runaways = nullptr;
    Girl* m_LastRunaway = nullptr;
    
    long m_BribeRate = 0;               // the amount of money spent bribing officials per week
    int m_Influence = 0;                // based on the bribe rate this is the percentage of influence you have
    int m_Dummy = 0;                    // a dummy variable
    long m_Bank = 0;                    // how much is stored in the bank
    
    Objective* m_Objective = nullptr;
    
    cRivalManager m_Rivals = {};        // all of the players compedators
    
    bool m_TortureDoneFlag = false;     // WD:  Have we got a torturer working today
    int m_Processing_Shift = -1;        // WD:  Store DayNight value when processing girls
};

} // namespace WhoreMasterRenewal

#endif // BROTHELMANAGER_HPP_INCLUDED_1636
