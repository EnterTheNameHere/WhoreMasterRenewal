#ifndef GIRLMANAGER_HPP_INCLUDED_2007
#define GIRLMANAGER_HPP_INCLUDED_2007

#include "Constants.h"
#include "cGirls.h" // required cImgageListManager
#include "cNameList.h" // required cNameList

#include <memory>
#include <string>
#include <vector>

class TiXmlElement;
class TiXmlHandle;

namespace WhoreMasterRenewal
{

class GirlManager;
extern GirlManager g_Girls;
class cAbstractGirls;
extern cAbstractGirls* g_GirlsPtr;



class Girl;
class sCustomer;
class sTrait;
//class sRandomGirl;
class Brothel;
//class sChild;
class CSurface;
class cAnimatedSurface;
typedef uint8_t Uint8; // SDL



class GirlPredicate
{
public:
    virtual ~GirlPredicate() {}
    virtual bool test( Girl* )
    {
        return true;
    }
};



class cAbstractGirls
{
public:
    virtual ~cAbstractGirls() {};
    
    virtual int GetStat( Girl* girl, int stat ) = 0;
    virtual int GetSkill( Girl* girl, int skill ) = 0;
    virtual void UpdateStat( Girl* girl, int stat, int amount ) = 0;
    virtual void UpdateSkill( Girl* girl, int skill, int amount ) = 0;
    virtual bool CalcPregnancy(
        Girl* girl, int chance, int type,
        unsigned char stats[NUM_STATS],
        unsigned char skills[NUM_SKILLS]
    ) = 0;
//  virtual void AddTrait(Girl* girl, std::string name, bool temp = false, bool removeitem = false, bool remember = false)=0;
    virtual bool AddTrait( Girl* girl, std::string name, bool temp = false, bool removeitem = false, bool remember = false ) = 0;
    virtual bool HasTrait( Girl* girl, std::string name ) = 0;
    virtual void UpdateTempSkill( Girl* girl, int skill, int amount ) = 0; // updates a skill temporarily
    virtual void UpdateTempStat( Girl* girl, int stat, int amount ) = 0;
};



// Keeps track of all the available (not used by player) girls in the game.
class GirlManager : public cAbstractGirls
{
public:
    GirlManager();
    virtual ~GirlManager();
    
    GirlManager( const GirlManager& ) = delete;
    GirlManager& operator = ( const GirlManager& ) = delete;
    
    void Free();
    
    void LoadDefaultImages();
    /*
     *  load the templated girls
     *  (if loading a save game doesn't load from the global template,
     *  loads from the save games' template)
     *
     *  LoadGirlsDecider is a wrapper function that decides to load
     *  XML or Legacy formats.
     *  LoadGirlsXML loads the XML files
     *  LoadGirlsLegacy is the original load function. More or less.
     */
    void LoadGirlsDecider( std::string filename );
    void LoadGirlsXML( std::string filename );
    void LoadGirlsLegacy( std::string filename );
    /*
     *  SaveGirls doesn't seem to be the inverse of LoadGirls
     *  but rather writes girl data to the save file
     */
    TiXmlElement* SaveGirlsXML( TiXmlElement* pRoot );  // Saves the girls to a file
    bool LoadGirlsXML( TiXmlHandle hGirls );
    void LoadGirlsLegacy( std::ifstream& ifs );
    void LoadGirlLegacy( Girl* current, std::ifstream& ifs );
    
    void AddGirl( Girl* girl );     // adds a girl to the list
    void RemoveGirl( Girl* girl, bool deleteGirl = false ); // Removes a girl from the list (only used with editor where all girls are available)
    
    Girl* GetGirl( int girl );  // gets the girl by count
    
    void GirlFucks( Girl* girl, int DayNight, sCustomer* customer, bool group, std::string& message, u_int& SexType );  // does the logic for fucking
    // MYR: Millions of ways to say, [girl] does [act] to [customer]
    std::string GetRandomGroupString();
    std::string GetRandomSexString();
    std::string GetRandomLesString();
    std::string GetRandomBDSMString();
    std::string GetRandomBeastString();
    std::string GetRandomAnalString();
    
    // MYR: More functions for attack/defense/agility-style combat.
    int GetCombatDamage( Girl* girl, int CombatType );
    int TakeCombatDamage( Girl* girl, int amt );
    
    void LevelUp( Girl* girl ); // advances a girls level
    void LevelUpStats( Girl* girl ); // Functionalized stat increase for LevelUp
    void UpdateStat( Girl* girl, int stat, int amount ); // updates a stat
    void LoadGirlImages( Girl* girl );  // loads a girls images using her name to check that directory in the characters folder
    void ApplyTraits( Girl* girl, sTrait* trait = nullptr, bool rememberflag = false ); // applys the stat bonuses for traits to a girl
    void UnapplyTraits( Girl* girl, sTrait* trait = nullptr );  // unapplys a trait (or all traits) from a girl
    bool PossiblyGainNewTrait( Girl* girl, std::string Trait, int Threshold, int ActionType, std::string Message, bool DayNight );
    //int UnapplyTraits(Girl* girl, sTrait* trait = nullptr);   // unapplys a trait (or all traits) from a girl
    void UpdateSkill( Girl* girl, int skill, int amount );  // updates a skill
    void UpdateEnjoyment( Girl* girl, int whatSheEnjoys, int amount, bool wrapTo100 = false ); //updates what she enjoys
    int DrawGirl( Girl* girl, int x, int y, int width, int height, int ImgType, bool random = true, int img = 0 );  // draws a image of a girl
    std::shared_ptr<CSurface> GetImageSurface( Girl* girl, int ImgType, bool random, int& img, bool gallery = false ); // draws a image of a girl
    cAnimatedSurface* GetAnimatedSurface( Girl* girl, int ImgType, int& img );
    bool IsAnimatedSurface( Girl* girl, int ImgType, int& img );
    bool HasTrait( Girl* girl, std::string trait );
    bool HasRememberedTrait( Girl* girl, std::string trait );
    int GetNumSlaveGirls();
    int GetNumCatacombGirls();
    int GetSlaveGirl( int from );
    int GetStat( Girl* girl, int stat );
    int GetSkill( Girl* girl, int skill );
    void SetSkill( Girl* girl, int skill, int amount );
    void SetStat( Girl* girl, int stat, int amount );
    void UpdateTempSkill( Girl* girl, int skill, int amount );  // updates a skill temporarily
    void UpdateTempStat( Girl* girl, int stat, int amount ); // updates a stat temporarily
    int GetRebelValue( Girl* girl, bool matron );
    void EquipCombat( Girl* girl ); // girl makes sure best armor and weapons are equipped, ready for combat
    void UnequipCombat( Girl* girl ); // girl unequips armor and weapons, ready for brothel work or other non-aggressive jobs
    bool RemoveInvByNumber( Girl* girl, int Pos );
    
    Uint8 girl_fights_girl( Girl* a, Girl* b );
    
    bool InheritTrait( sTrait* trait );
    
    void AddRandomGirl( sRandomGirl* girl );
    /*
     *  mod - docclox
     *  same deal here: LoadRandomGirl is a wrapper
     *  The "-Legacy" version is the original
     *  The "-XML" version is the new one that loads from XML files
     */
    void LoadRandomGirl( std::string filename );
    void LoadRandomGirlXML( std::string filename );
    void LoadRandomGirlLegacy( std::string filename );
    // end mod
    
    Girl* CreateRandomGirl( int age, bool addToGGirls, bool slave = false, bool undead = false, bool NonHuman = false, bool childnaped = false );
    
    Girl* GetRandomGirl( bool slave = false, bool catacomb = false );
    
    bool NameExists( std::string name );
    
    bool CheckInvSpace( Girl* girl );
    int AddInv( Girl* girl, sInventoryItem* item );
    bool EquipItem( Girl* girl, int num, bool force );
    bool CanEquip( Girl* girl, int num, bool force );
    int GetWorseItem( Girl* girl, int type, int cost );
    int GetNumItemType( Girl* girl, int Type );
    void SellInvItem( Girl* girl, int num );
    void UseItems( Girl* girl );
    int HasItem( Girl* girl, std::string name );
//  void RemoveTrait(Girl* girl, std::string name, bool addrememberlist = false, bool force = false);
    bool RemoveTrait( Girl* girl, std::string name, bool addrememberlist = false, bool force = false );
    void RemoveRememberedTrait( Girl* girl, std::string name );
    void RemoveAllRememberedTraits( Girl* girl );               // WD: Cleanup remembered traits on new girl creation
    int GetNumItemEquiped( Girl* girl, int Type );
    bool IsItemEquipable( Girl* girl, int num );
    bool IsInvFull( Girl* girl );
    
    int GetSkillWorth( Girl* girl );
    
    bool DisobeyCheck( Girl* girl, int action, Brothel* brothel = nullptr );
    
    std::string GetDetailsString( Girl* girl, bool purchace = false );
    std::string GetMoreDetailsString( Girl* girl );
    std::string GetGirlMood( Girl* girl );
    
//  void AddTrait(Girl* girl, std::string name, bool temp = false, bool removeitem = false, bool inrememberlist = false);
    bool AddTrait( Girl* girl, std::string name, bool temp = false, bool removeitem = false, bool inrememberlist = false );
    void AddRememberedTrait( Girl* girl, std::string name );
    
    cImgageListManager* GetImgManager()
    {
        return &m_ImgListManager;
    }
    
    void CalculateAskPrice( Girl* girl, bool vari );
    
    void AddTiredness( Girl* girl );
    
    void SetAntiPreg( Girl* girl, bool useAntiPreg );
    
    bool GirlInjured( Girl* girl, unsigned int modifier );
    
    void CalculateGirlType( Girl* girl ); // updates a girls fetish type based on her traits and stats
    bool CheckGirlType( Girl* girl, int type ); // Checks if a girl has this fetish type
    
    void do_abnormality( Girl* sprog, int chance );
    void HandleChild( Girl* girl, sChild* child, std::string& summary );
    void HandleChild_CheckIncest( Girl* mum, Girl* sprog, sChild* child, std::string& summary );
    bool child_is_grown( Girl* girl, sChild* child, std::string& summary, bool PlayerControlled = true );
    bool child_is_due( Girl* girl, sChild* child, std::string& summary, bool PlayerControlled = true );
    void HandleChildren( Girl* girl, std::string& summary, bool PlayerControlled = true );  // ages children and handles pregnancy
    bool CalcPregnancy( Girl* girl, int chance, int type, unsigned char stats[NUM_STATS], unsigned char skills[NUM_SKILLS] ); // checks if a girl gets pregnant
    void UncontrolledPregnancies(); // ages children and handles pregnancy for all girls not controlled by player
    
    // mod - docclox - func to return random girl N in the chain
    // returns null if n out of range
    sRandomGirl* random_girl_at( u_int n );
    /*
     *  while I'm on, a few funcs to factor out some common code in DrawImages
     */
    int num_images( Girl* girl, int image_type );
    int get_modified_image_type( Girl* girl, int image_type, int preg_type );
    int draw_with_default(
        Girl* girl,
        int x, int y,
        int width, int height,
        int ImgType,
        bool random,
        int img
    );
    int calc_abnormal_pc( Girl* mom, Girl* sprog, bool is_players );
    
    std::vector<Girl*>  get_girls( GirlPredicate* pred );
    
    // end mod
    
    // WD:  Consolidate common code in BrothelUpdate and DungeonUpdate to fn's
    void updateGirlAge( Girl* girl, bool inc_inService = false );
    void updateTempStats( Girl* girl );
    void updateTempSkills( Girl* girl );
    void updateTempTraits( Girl* girl );
    void updateSTD( Girl* girl );
    void updateHappyTraits( Girl* girl );
    void updateGirlTurnStats( Girl* girl );
    
    
    
private:
    unsigned int m_NumGirls = 0;    // number of girls in the class
    Girl* m_Parent = nullptr; // first in the list of girls who are dead, gone or in use
    Girl* m_Last = nullptr;   // last in the list of girls who are dead, gone or in use
    
    unsigned int m_NumRandomGirls = 0;
    sRandomGirl* m_RandomGirls = nullptr;
    sRandomGirl* m_LastRandomGirls = nullptr;
    
    // These are the default images used when a character is missing images for that particular purpose
    cAImgList* m_DefImages = nullptr;
    cImgageListManager m_ImgListManager = {};
    cNameList m_Names;
};

} // namespace WhoreMasterRenewal

#endif // GIRLMANAGER_HPP_INCLUDED_2007
