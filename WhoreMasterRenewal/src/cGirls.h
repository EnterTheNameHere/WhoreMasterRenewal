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
#ifndef CGIRL_H_INCLUDED_1528
#define CGIRL_H_INCLUDED_1528
#pragma once

#include "Constants.h"
#include "cEvents.h" // required cEvents
#include "cTriggers.h" // required cTriggerList
#include "cNameList.h" // required cNameList
#include "Girl.hpp" // required for various reasons

#include <map>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

class TiXmlElement;

namespace WhoreMasterRenewal
{

class cGirls;
extern cGirls g_Girls;
class cAbstractGirls;
extern cAbstractGirls* g_GirlsPtr;
class Girl;
extern Girl* MarketSlaveGirls[8];
extern int MarketSlaveGirlsDel[8];
extern Girl* selected_girl;
extern std::vector<int> cycle_girls;
extern int cycle_pos;

class cIndexedList;
class cPlayer;
class cAnimatedSurface;
class CSurface;
struct sCustomer;
struct sGang;
struct sInventoryItem;
struct sBrothel;
struct sTrait;

typedef uint8_t Uint8; // Used by SDL

bool has_contraception( Girl* girl );

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



// structure to hold randomly generated girl information
typedef struct sRandomGirl
{
    std::string m_Name;
    std::string m_Desc;
    
    unsigned char m_Human;                      // 0 means they are human otherwise they are not
    unsigned char m_Catacomb;                   // 1 means they are a monster found in catacombs, 0 means wanderer
    
    unsigned char m_MinStats[NUM_STATS];        // min and max stats they may start with
    unsigned char m_MaxStats[NUM_STATS];
    
    unsigned char m_MinSkills[NUM_SKILLS];      // min and max skills they may start with
    unsigned char m_MaxSkills[NUM_SKILLS];
    
    unsigned char m_NumTraits;                  // number of traits they are assigned
    sTrait* m_Traits[MAXNUM_TRAITS];            // List of traits they may start with
    unsigned char m_TraitChance[MAXNUM_TRAITS]; // the percentage change for each trait
    
    int m_MinMoney; // min and max money they can start with
    int m_MaxMoney;
    
    sRandomGirl* m_Next;
    /*
     *  MOD: DocClox Sun Nov 15 06:11:43 GMT 2009
     *  stream operator for debugging
     *  plus a shitload of XML loader funcs
     */
    friend std::ostream& operator<<( std::ostream& os, sRandomGirl& g );
    /*
     *  one func to load the girl node,
     *  and then one each for each embedded node
     *
     *  Not so much difficult as tedious.
     */
    void load_from_xml( TiXmlElement* );
    void process_trait_xml( TiXmlElement* );
    void process_stat_xml( TiXmlElement* );
    void process_skill_xml( TiXmlElement* );
    void process_cash_xml( TiXmlElement* );
    /*
     *  END MOD
     */
    static Girl* lookup;  // used to look up stat and skill IDs
    sRandomGirl();
    ~sRandomGirl();
} sRandomGirl;



// Character image
class cImage
{
public:
    cImage();
    ~cImage();
    
    cImage* m_Next;
    std::shared_ptr<CSurface> m_Surface;
    cAnimatedSurface* m_AniSurface;
};

// Character image management class
class cImageList
{
public:
    cImageList();
    ~cImageList();
    
    void Free();
    
    bool AddImage( std::string filename, std::string path = "", std::string file = "" );
    int DrawImage( int x, int y, int width, int height, bool random, int img );
    std::shared_ptr<CSurface> GetImageSurface( bool random, int& img );
    cAnimatedSurface* GetAnimatedSurface( int& img );
    bool IsAnimatedSurface( int& img );
    std::string GetName( int i );
    
    int m_NumImages;
    cImage* m_Images;
    cImage* m_LastImages;
};

class cAImgList // class that manages a set of images from a directory
{
public:
    cAImgList();
    ~cAImgList();
    
    std::string m_Name; // name of the directory containing the images
    cImageList m_Images[NUM_IMGTYPES];  // the images
    
    cAImgList* m_Next;
};


class cImgageListManager    // manages all the girl images
{
public:
    cImgageListManager();
    ~cImgageListManager();
    
    void Free();
    
    cAImgList* ListExists( std::string name );  // returns the list if the list is already loaded, returns 0 if it is not
    cAImgList* LoadList( std::string name ); // loads a list if it doensn't already exist and returns a pointer to it. returns pointer to list if it does exist
    
private:
    cAImgList* m_First;
    cAImgList* m_Last;
};

typedef struct sChild
{
    enum Gender
    {
        None    = -1,
        Girl    =  0,
        Boy =  1
    };
    Gender m_Sex;
    
    std::string boy_girl_str()
    {
        if( m_Sex == Boy )
            return "boy";
            
        return "girl";
    }
    bool is_boy()
    {
        return m_Sex == Boy;
    }
    bool is_girl()
    {
        return m_Sex == Girl;
    }
    
    unsigned char m_Age;    // grows up at 60 weeks
    unsigned char m_IsPlayers;  // 1 when players
    unsigned char m_Unborn; // 1 when child is unborn (for when stats are inherited from customers)
    
    // skills and stats from the father
    unsigned char m_Stats[NUM_STATS];
    unsigned char m_Skills[NUM_SKILLS];
    
    sChild* m_Next;
    sChild* m_Prev;
    
    sChild( bool is_players = false, Gender gender = None );
    ~sChild();
    
    TiXmlElement* SaveChildXML( TiXmlElement* pRoot );
    bool LoadChildXML( TiXmlHandle hChild );
    
} sChild;

class GirlPredicate
{
public:
    virtual ~GirlPredicate() {}
    virtual bool test( Girl* )
    {
        return true;
    }
};

// Keeps track of all the available (not used by player) girls in the game.
class cGirls : public cAbstractGirls
{
public:
    cGirls();
    virtual ~cGirls();
    
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
    
    bool DisobeyCheck( Girl* girl, int action, sBrothel* brothel = nullptr );
    
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
    unsigned int m_NumGirls;    // number of girls in the class
    Girl* m_Parent; // first in the list of girls who are dead, gone or in use
    Girl* m_Last;   // last in the list of girls who are dead, gone or in use
    
    unsigned int m_NumRandomGirls;
    sRandomGirl* m_RandomGirls;
    sRandomGirl* m_LastRandomGirls;
    
    // These are the default images used when a character is missing images for that particular purpose
    cAImgList* m_DefImages;
    cImgageListManager m_ImgListManager;
    cNameList names;
};

} // namespace WhoreMasterRenewal

#endif  // CGIRL_H_INCLUDED_1528
