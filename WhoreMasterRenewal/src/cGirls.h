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

#include "Constants.h"

#include <map>
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

class TiXmlElement;
class TiXmlHandle;

namespace WhoreMasterRenewal
{

class Girl;
class cIndexedList;
class cPlayer;
class cAnimatedSurface;
class CSurface;
struct sCustomer;
struct sGang;
struct sInventoryItem;
class Brothel;
struct sTrait;

typedef uint8_t Uint8; // Used by SDL

bool has_contraception( Girl* girl );



// structure to hold randomly generated girl information
typedef struct sRandomGirl
{
public:
    sRandomGirl();
    ~sRandomGirl();
    
    sRandomGirl( const sRandomGirl& ) = delete;
    sRandomGirl& operator = ( const sRandomGirl& ) = delete;
    
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
    
    friend std::ostream& operator<<( std::ostream& os, sRandomGirl& g );
    
    
    
    std::string m_Name = "";
    std::string m_Desc = "";
    
    unsigned char m_Human = 0;                      // 0 means they are human otherwise they are not
    unsigned char m_Catacomb = 0;                   // 1 means they are a monster found in catacombs, 0 means wanderer
    
    unsigned char m_MinStats[NUM_STATS];        // min and max stats they may start with
    unsigned char m_MaxStats[NUM_STATS];
    
    unsigned char m_MinSkills[NUM_SKILLS];      // min and max skills they may start with
    unsigned char m_MaxSkills[NUM_SKILLS];
    
    unsigned char m_NumTraits = 0;                  // number of traits they are assigned
    sTrait* m_Traits[MAXNUM_TRAITS];            // List of traits they may start with
    unsigned char m_TraitChance[MAXNUM_TRAITS]; // the percentage change for each trait
    
    int m_MinMoney = 0; // min and max money they can start with
    int m_MaxMoney = 0;
    
    sRandomGirl* m_Next = nullptr;
    
    static Girl* lookup;  // used to look up stat and skill IDs
} sRandomGirl;



// Character image
class cImage
{
public:
    cImage();
    ~cImage();
    
    cImage( const cImage& ) = delete;
	cImage& operator = ( const cImage& ) = delete;
    
    cImage* m_Next = nullptr;
    std::shared_ptr<CSurface> m_Surface = nullptr;
    cAnimatedSurface* m_AniSurface = nullptr;
};

// Character image management class
class cImageList
{
public:
    cImageList();
    ~cImageList();
    
    cImageList( const cImageList& ) = delete;
    cImageList& operator = ( const cImageList& ) = delete;
    
    void Free();
    
    bool AddImage( std::string filename, std::string path = "", std::string file = "" );
    int DrawImage( int x, int y, int width, int height, bool random, int img );
    std::shared_ptr<CSurface> GetImageSurface( bool random, int& img );
    cAnimatedSurface* GetAnimatedSurface( int& img );
    bool IsAnimatedSurface( int& img );
    std::string GetName( int i );
    
    int m_NumImages = 0;
    cImage* m_Images = nullptr;
    cImage* m_LastImages = nullptr;
};

class cAImgList // class that manages a set of images from a directory
{
public:
    cAImgList();
    ~cAImgList();
    
    cAImgList( const cAImgList& ) = delete;
	cAImgList& operator = ( const cAImgList& ) = delete;
    
    std::string m_Name = ""; // name of the directory containing the images
    cImageList m_Images[NUM_IMGTYPES];  // the images
    
    cAImgList* m_Next = nullptr;
};


class cImgageListManager    // manages all the girl images
{
public:
    cImgageListManager();
    ~cImgageListManager();
    
    cImgageListManager( const cImgageListManager& ) = delete;
    cImgageListManager& operator = ( const cImgageListManager& ) = delete;
    
    void Free();
    
    cAImgList* ListExists( std::string name );  // returns the list if the list is already loaded, returns 0 if it is not
    cAImgList* LoadList( std::string name ); // loads a list if it doensn't already exist and returns a pointer to it. returns pointer to list if it does exist
    
private:
    cAImgList* m_First = nullptr;
    cAImgList* m_Last = nullptr;
};

typedef struct sChild
{
public:
    enum Gender
    {
        None = -1,
        Girl =  0,
        Boy =  1
    };
    
    sChild( bool is_players = false, Gender gender = Gender::None ); // Gender::None means random
    ~sChild();
    
    sChild( const sChild& ) = delete;
    sChild& operator = ( const sChild& ) = delete;
    
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
    
    TiXmlElement* SaveChildXML( TiXmlElement* pRoot );
    bool LoadChildXML( TiXmlHandle hChild );
    
    unsigned char m_Age = 0;    // grows up at 60 weeks
    unsigned char m_IsPlayers;  // 1 when players
    unsigned char m_Unborn = 1; // 1 when child is unborn (for when stats are inherited from customers)
    
    Gender m_Sex;
    
    // skills and stats from the father
    unsigned char m_Stats[NUM_STATS];
    unsigned char m_Skills[NUM_SKILLS];
    
    sChild* m_Next = nullptr;
    sChild* m_Prev = nullptr;
} sChild;

} // namespace WhoreMasterRenewal

#endif  // CGIRL_H_INCLUDED_1528
