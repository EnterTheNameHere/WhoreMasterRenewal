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

#include "cGirls.h"
#include "GirlManager.hpp"
#include "cTariff.h"
#include "cEvents.h"
#include "math.h"
#include "Brothel.hpp"
#include "cMessageBox.h"
#include "cGold.h"
#include "cGangs.h"
#include "CGraphics.h"
#include "DirPath.h"
#include "FileList.h"
#include "Helper.hpp"
#include "XmlMisc.h"
#include "cAnimatedSurface.h"
#include "cTraits.h"
#include "cCustomers.h"
#include "CSurface.h"
#include "cInventory.h"
#include "CLog.h"
#include "BrothelManager.hpp"
#include "cRng.h"
#include "InterfaceProcesses.h"
#include "Girl.hpp"

#include <fstream>
#include <algorithm>
#include <climits>

namespace WhoreMasterRenewal
{

bool has_contraception( Girl* girl )
{
    /*
     *  if contraception is TRUE, then she can't get pregnant
     *  Which makes sense
     *
     *  Two ways to get contraception: one of them is
     *  if the brothel has a an anti-preg potion
     *  (and if she's willing to drink it!)
     *
     *  this tests both
     */
    if( g_Brothels.UseAntiPreg( girl->m_UseAntiPreg ) )
    {
        return true;
    }
    
    /*
     *  otherwise, sterile babes should be safe as well
     */
    if( girl->has_trait( "Sterile" ) )
    {
        return true;
    }
    
    /*
     *  Actually, I lied. If she's in her cooldown period after
     *  giving birth, that is effective contraception as well
     */
    if( girl->m_PregCooldown > 0 )
    {
        return true;
    }
    
    /*
     *  Oh, and if she's pregnant, she shouldn't get pregnant
     *  So I guess that's four
     */
    if( girl->is_pregnant() )
    {
        return true;
    }
    
    return false;
}

// ----- Utility

static char* n_strdup( const char* s )
{
    return strcpy( new char[strlen( s ) + 1], s );
}

Girl* sRandomGirl::lookup = new Girl();  // used to look up stat and skill IDs

// ----- Misc

bool sChild::LoadChildXML( TiXmlHandle hChild )
{
    TiXmlElement* pChild = hChild.ToElement();
    
    if( pChild == nullptr )
    {
        return false;
    }
    
    int tempInt = 0;
    pChild->QueryIntAttribute( "Age", &tempInt );
    m_Age = tempInt;
    tempInt = 0;
    pChild->QueryIntAttribute( "IsPlayers", &tempInt );
    m_IsPlayers = tempInt;
    tempInt = 0;
    pChild->QueryIntAttribute( "Sex", &tempInt );
    m_Sex = sChild::Gender( tempInt );
    tempInt = 0;
    pChild->QueryIntAttribute( "Unborn", &tempInt );
    m_Unborn = tempInt;
    tempInt = 0;
    
    // load their stats
    LoadStatsXML( hChild.FirstChild( "Stats" ), m_Stats );
    
    // load their skills
    LoadSkillsXML( hChild.FirstChild( "Skills" ), m_Skills );
    return true;
}

TiXmlElement* sChild::SaveChildXML( TiXmlElement* pRoot )
{
    TiXmlElement* pChild = new TiXmlElement( "Child" );
    pRoot->LinkEndChild( pChild );
    pChild->SetAttribute( "Age", m_Age );
    pChild->SetAttribute( "IsPlayers", m_IsPlayers );
    pChild->SetAttribute( "Sex", m_Sex );
    pChild->SetAttribute( "Unborn", m_Unborn );
    // Save their stats
    SaveStatsXML( pChild, m_Stats );
    
    // save their skills
    SaveSkillsXML( pChild, m_Skills );
    return pChild;
}

sRandomGirl::sRandomGirl()
{
    //assigning defaults
    for( int i = 0; i < NUM_STATS; i++ )
    {
        m_MinStats[i] = 30;
        m_MaxStats[i] = 60;
    }
    
    for( int i = 0; i < 10; i++ )
    {
        m_MinSkills[i] = 30;
        m_MaxSkills[i] = 30;
    }
    
    //now for a few overrides
    m_MinStats[STAT_AGE] = 17;
    m_MaxStats[STAT_AGE] = 25;
    m_MinStats[STAT_HOUSE] = 0;
    m_MaxStats[STAT_HOUSE] = 100;
    m_MinStats[STAT_HEALTH] = 100;
    m_MaxStats[STAT_HEALTH] = 100;
    m_MinStats[STAT_FAME] = 0;
    m_MaxStats[STAT_FAME] = 0;
    m_MinStats[STAT_LEVEL] = 0;
    m_MaxStats[STAT_LEVEL] = 0;
    m_MinStats[STAT_PCFEAR] = 0;
    m_MaxStats[STAT_PCFEAR] = 0;
    m_MinStats[STAT_PCHATE] = 0;
    m_MaxStats[STAT_PCHATE] = 0;
    m_MinStats[STAT_PCLOVE] = 0;
    m_MaxStats[STAT_PCLOVE] = 0;
}

sRandomGirl::~sRandomGirl()
{
    if( m_Next )delete m_Next;
    
    m_Next = nullptr;
}

void sRandomGirl::load_from_xml( TiXmlElement* el )
{
    const char* pt;
    m_NumTraits = 0;
    /*
     *  name and description are easy
     */
    pt = el->Attribute( "Name" );
    
    if( pt )
        m_Name = pt;
        
    g_LogFile.ss() << "Loading " << pt << std::endl;
    g_LogFile.ssend();
    
    if( ( pt = el->Attribute( "Desc" ) ) )
        m_Desc = pt;
        
    /*
     *  logic is back-to-front on this one...
     */
    if( ( pt = el->Attribute( "Human" ) ) )
    {
        if( strcmp( pt, "Yes" ) == 0 )
            m_Human = 1;
        else
            m_Human = 0;
    }
    
    /*
     *  catacomb dweller?
     */
    if( ( pt = el->Attribute( "Catacomb" ) ) )
    {
        if( strcmp( pt, "Yes" ) == 0 )
            m_Catacomb = 1;
        else
            m_Catacomb = 0;
    }
    
    /*
     *  loop through children
     */
    TiXmlElement* child;
    
    for( child = el->FirstChildElement(); child ; child = child->NextSiblingElement() )
    {
        /*
         *      now: what we do depends on the tag string
         *      which we can get from the ValueStr() method
         *
         *      Let's process each tag type in its own method.
         *      Keep things cleaner that way.
         */
        if( child->ValueStr() == "Gold" )
        {
            process_cash_xml( child );
            continue;
        }
        
        /*
         *      OK: is it a stat?
         */
        if( child->ValueStr() == "Stat" )
        {
            process_stat_xml( child );
            continue;
        }
        
        /*
         *      How about a skill?
         */
        if( child->ValueStr() == "Skill" )
        {
            process_skill_xml( child );
            continue;
        }
        
        /*
         *      surely a trait then?
         */
        if( child->ValueStr() == "Trait" )
        {
            process_trait_xml( child );
            continue;
        }
        
        /*
         *      None of the above? Better ask for help then.
         */
        g_LogFile.ss() << "Unexpected tag: " << child->ValueStr() << std::endl
                       << "	don't know what do to, ignoring" << std::endl;
        g_LogFile.ssend();
    }
}

// ----- Tag processing

void sRandomGirl::process_trait_xml( TiXmlElement* el )
{
    int ival;
    const char* pt;
    /*
     *  we need to allocate a new sTrait scruct,
     */
    sTrait* trait = new sTrait();
    
    /*
     *  get the trait name
     */
    if( ( pt = el->Attribute( "Name" ) ) )
    {
        trait->m_Name = n_strdup( pt );
    }
    
    /*
     *  store that in the next free index slot
     */
    m_Traits[m_NumTraits] = trait;
    
    /*
     *  get the percentage chance
     */
    if( ( pt = el->Attribute( "Percent", &ival ) ) )
    {
        m_TraitChance[m_NumTraits] = ival;
    }
    
    /*
     *  and whack up the trait count.
     */
    m_NumTraits ++;
}

void sRandomGirl::process_stat_xml( TiXmlElement* el )
{
    int ival, index;
    const char* pt;
    
    if( ( pt = el->Attribute( "Name" ) ) )
    {
        index = lookup->stat_lookup[pt];
    }
    else
    {
        g_LogFile.ss() << "can't find 'Name' attribute - can't process stat"
                       << std::endl;
        g_LogFile.ssend();
        return;     // do as much as we can without crashing
    }
    
    if( ( pt = el->Attribute( "Min", &ival ) ) )
    {
        m_MinStats[index] = ival;
    }
    
    if( ( pt = el->Attribute( "Max", &ival ) ) )
    {
        m_MaxStats[index] = ival;
    }
}

void sRandomGirl::process_skill_xml( TiXmlElement* el )
{
    int ival, index;
    const char* pt;
    
    /*
     *  Strictly, I should use something that lets me
     *  test for absence. This won't catch typos in the
     *  XML file
     */
    if( ( pt = el->Attribute( "Name" ) ) )
    {
        index = lookup->skill_lookup[pt];
    }
    else
    {
        g_LogFile.ss() << "can't find 'Name' attribute - can't process skill"
                       << std::endl;
        g_LogFile.ssend();
        return;     // do as much as we can without crashing
    }
    
    if( ( pt = el->Attribute( "Min", &ival ) ) )
    {
        m_MinSkills[index] = ival;
    }
    
    if( ( pt = el->Attribute( "Max", &ival ) ) )
    {
        m_MaxSkills[index] = ival;
    }
}

void sRandomGirl::process_cash_xml( TiXmlElement* el )
{
    int ival;
    const char* pt;
    
    if( ( pt = el->Attribute( "Min", &ival ) ) )
    {
        g_LogFile.ss() << "	min money = " << ival << std::endl;
        g_LogFile.ssend();
        m_MinMoney = ival;
    }
    
    if( ( pt = el->Attribute( "Max", &ival ) ) )
    {
        g_LogFile.ss() << "	max money = " << ival << std::endl;
        g_LogFile.ssend();
        m_MaxMoney = ival;
    }
}


// ----- Stream operators

/*
 * While I'm here, let's defined the stream operator for the girls,
 * random and otherwise.
 *
 * This looks big and complex, but it's just printing out the random
 * girl data from the load. It's so I can say std::cout << *rgirl << std::endl;
 * and get a something sensible printed
 */

std::ostream& operator<<( std::ostream& os, sRandomGirl& g )
{
    os << g.m_Name << std::endl;
    os << g.m_Desc << std::endl;
    os << "Human? " << ( g.m_Human == 0 ? "Yes" : "No" ) << std::endl;
    os << "Catacomb Dweller? "
       << ( g.m_Catacomb == 0 ? "No" : "Yes" )
       << std::endl;
    os << "Money: Min = " << g.m_MinMoney << ". Max = " << g.m_MaxMoney << std::endl;
    
    /*
     *  loop through stats
     *  setw sets a field width for the next operation,
     *  left forces left alignment. Makes the columns line up.
     */
    for( unsigned int i = 0; i < Girl::max_stats; i++ )
    {
        os << std::setw( 14 ) << std::left << Girl::stat_names[i]
           << ": Min = " << int( g.m_MinStats[i] )
           << std::endl
           ;
        os << std::setw( 14 ) << ""
           << ": Max = " << int( g.m_MaxStats[i] )
           << std::endl
           ;
    }
    
    /*
     *  loop through skills
     */
    for( unsigned int i = 0; i < Girl::max_skills; i++ )
    {
        os << std::setw( 14 ) << std::left << Girl::skill_names[i]
           << ": Min = " << int( g.m_MinSkills[i] )
           << std::endl
           ;
        os << std::setw( 14 ) << ""
           << ": Max = " << int( g.m_MaxSkills[i] )
           << std::endl
           ;
    }
    
    /*
     *  loop through traits
     */
    for( int i = 0; i < g.m_NumTraits; i++ )
    {
        std::string name = g.m_Traits[i]->m_Name;
        int percent = int( g.m_TraitChance[i] );
        os << "Trait: "
           << std::setw( 14 ) << std::left << name
           << ": " << percent
           << "%"
           << std::endl
           ;
    }
    
    /*
     *  important to return the stream, so the next
     *  thing in the << chain has something on which to operate
     */
    return os;
}

sChild::sChild( bool is_players, Gender gender )
    : m_IsPlayers(is_players),
    m_Sex(gender)
{
    
    if( gender != None )
    {
        return;
    }
    
    m_Sex = ( g_Dice.is_girl() ? Gender::Girl : Gender::Boy );
}

sChild::~sChild()
{
    m_Prev = nullptr;
    
    if( m_Next )delete m_Next;
    
    m_Next = nullptr;
}

cAImgList::cAImgList()
{
    ;
}

cAImgList::~cAImgList()
{
    //g_LogFile.ss() << "cAImgList::~cAImgList() [" << this << "] name: \"" << m_Name << "\"";
    //g_LogFile.ssend();
    
    for( int i = 0; i < NUM_IMGTYPES; i++ )
        m_Images[i].Free();
        
    if( m_Next )
        delete m_Next;
        
    m_Next = nullptr;
    
    //g_LogFile.ss() << "cAImgList::~cAImgList() finished [" << this << "] name: \"" << m_Name << "\"";
    //g_LogFile.ssend();
}

cImageList::cImageList()
{
    ;
}

cImageList::~cImageList()
{
    Free();
}

void cImageList::Free()
{
    //g_LogFile.ss() << "cImageList::~Free() [" << this << "]";
    //g_LogFile.ssend();
    
    if( m_Images )
        delete m_Images;
        
    m_LastImages = nullptr;
    m_Images = nullptr;
    m_NumImages = 0;
    
    //g_LogFile.ss() << "cImageList::~Free() finished [" << this << "]";
    //g_LogFile.ssend();
}

bool cImageList::AddImage( std::string filename, std::string path, std::string file )
{
    //std::ifstream in;
    //in.open(filename.c_str());
    //if(!in)
    //{
    //  in.close();
    //  return false;
    //}
    //in.close();
    
    // create image item
    cImage* newImage = new cImage();
    
    if( filename[filename.size() - 1] == 'i' )
    {
        std::string name = path;
        name += "\\ani\\";
        name += file;
        name.erase( name.size() - 4, 4 );
        name += ".jpg";
        newImage->m_Surface.reset( new CSurface() );
        
        newImage->m_Surface->LoadImage( name );
        newImage->m_AniSurface = new cAnimatedSurface();
        int numFrames, speed, aniwidth, aniheight;
        std::ifstream input;
        input.open( filename.c_str() );
        
        if( !input )
        {
            g_LogFile.ss() << "Incorrect data file given for animation - " << filename;
            g_LogFile.ssend();
            return false;
        }
        else
            input >> numFrames >> speed >> aniwidth >> aniheight;
            
        newImage->m_AniSurface->SetData( 0, 0, numFrames, speed, aniwidth, aniheight, newImage->m_Surface );
        input.close();
        //newImage->m_Surface->FreeResources();  //this was causing lockup in CResourceManager::CullOld
    }
    else
        newImage->m_Surface.reset( new CSurface( filename ) );
        
    // Store image item
    if( m_Images )
    {
        m_LastImages->m_Next = newImage;
        m_LastImages = newImage;
    }
    else
        m_LastImages = m_Images = newImage;
        
    return true;
}

std::shared_ptr<CSurface> cImageList::GetImageSurface( bool random, int& img )
{
    int count = 0;
    int ImageNum = -1;
    
    if( !random )
    {
        if( img == -1 )
            return nullptr;
            
        ImageNum = img;
        cImage* current = m_Images;
        
        while( current )
        {
            if( count == ImageNum )
                break;
                
            count++;
            current = current->m_Next;
        }
        
        if( current )
        {
            img = ImageNum;
            return current->m_Surface;
        }
    }
    else
    {
        if( m_NumImages == 0 )
            return nullptr;
        else if( m_NumImages == 1 )
        {
            img = 0;
            return m_Images->m_Surface;
        }
        else
        {
            ImageNum = g_Dice % m_NumImages;
            cImage* current = m_Images;
            
            while( current )
            {
                if( count == ImageNum )
                    break;
                    
                count++;
                current = current->m_Next;
            }
            
            if( current )
            {
                img = ImageNum;
                return current->m_Surface;
            }
            else
            {
                img = ImageNum;
                return nullptr;
            }
        }
    }
    
    img = ImageNum;
    return nullptr;
}

cAnimatedSurface* cImageList::GetAnimatedSurface( int& img )
{
    int count = 0;
    int ImageNum = -1;
    
    if( img == -1 )
        return nullptr;
        
    ImageNum = img;
    cImage* current = m_Images;
    
    while( current )
    {
        if( count == ImageNum )
            break;
            
        count++;
        current = current->m_Next;
    }
    
    if( current )
    {
        img = ImageNum;
        
        if( current->m_AniSurface )
            return current->m_AniSurface;
        else
            return nullptr;
    }
    
    return nullptr;
}

bool cImageList::IsAnimatedSurface( int& img )
{
    int count = 0;
    int ImageNum = -1;
    
    if( img == -1 )
        return false;
        
    ImageNum = img;
    cImage* current = m_Images;
    
    while( current )
    {
        if( count == ImageNum )
            break;
            
        count++;
        current = current->m_Next;
    }
    
    if( current )
    {
        img = ImageNum;
        
        if( current->m_AniSurface )
            return true;
        else
            return false;
    }
    
    return false;
}

int cImageList::DrawImage( int x, int y, int width, int height, bool random, int img )
{
    int count = 0;
    SDL_Rect rect;
    int ImageNum = -1;
    
    rect.y = rect.x = 0;
    rect.w = width;
    rect.h = height;
    
    if( !random )
    {
        if( img == -1 )
            return -1;
            
        if( img > m_NumImages )
        {
            if( m_NumImages == 1 )
                ImageNum = 0;
            else
                ImageNum = g_Dice % m_NumImages;
        }
        else
            ImageNum = img;
            
        cImage* current = m_Images;
        
        while( current )
        {
            if( count == ImageNum )
                break;
                
            count++;
            current = current->m_Next;
        }
        
        if( current )
        {
            if( current->m_AniSurface )
                current->m_AniSurface->DrawFrame( x, y, rect.w, rect.h, g_Graphics.GetTicks() );
            else
                current->m_Surface->DrawSurface( x, y, nullptr, &rect, true );
        }
    }
    else
    {
        if( m_NumImages == 0 )
            return -1;
        else if( m_NumImages == 1 )
        {
            m_Images->m_Surface->DrawSurface( x, y, nullptr, &rect, true );
            return 0;
        }
        else
        {
            ImageNum = g_Dice % m_NumImages;
            cImage* current = m_Images;
            
            while( current )
            {
                if( count == ImageNum )
                    break;
                    
                count++;
                current = current->m_Next;
            }
            
            if( current )
            {
                if( current->m_AniSurface )
                    current->m_AniSurface->DrawFrame( x, y, rect.w, rect.h, g_Graphics.GetTicks() );
                else
                    current->m_Surface->DrawSurface( x, y, nullptr, &rect, true );
            }
            else
                return -1;
        }
    }
    
    return ImageNum;
}

std::string cImageList::GetName( int i )
{
    int count = 0;
    cImage* current = m_Images;
    
    while( current )
    {
        if( count == i )
            break;
            
        count++;
        current = current->m_Next;
    }
    
    if( current )
        return current->m_Surface->GetFilename();
        
    return std::string( "" );
}

cImgageListManager::cImgageListManager()
{
    ;
}

cImgageListManager::~cImgageListManager()
{
    Free();
}

void cImgageListManager::Free()
{
    //g_LogFile.ss() << "cImageList::~Free() [" << this << "]\"";
    //g_LogFile.ssend();
    
    if( m_First )
        delete m_First;
    
    m_Last = nullptr;
    m_First = nullptr;
    
    //g_LogFile.ss() << "cImageList::~Free() finished [" << this << "]\"";
    //g_LogFile.ssend();
}

cAImgList* cImgageListManager::ListExists( std::string name )
{
    cAImgList* current = m_First;
    
    while( current )
    {
        if( current->m_Name == name )
            break;
            
        current = current->m_Next;
    }
    
    return current;
}

cAImgList* cImgageListManager::LoadList( std::string name )
{
    cAImgList* current = ListExists( name );
    
    if( current )
        return current;
        
    current = new cAImgList();
    current->m_Name = name;
    current->m_Next = nullptr;
    /* mod
    uses dir path and file list to construct the girl images
    */
    
    DirPath imagedir;
    imagedir << "Resources" << "Characters" << name;
    std::string numeric = "123456789";
    std::string pic_types[] = {"Anal*.jp*g", "BDSM*.jp*g", "Sex*.jp*g", "Beast*.jp*g", "Group*.jp*g", "Les*.jp*g", "Preg*.jp*g", "Death*.jp*g", "Profile*.jp*g", "PregAnal*.jp*g", "PregBDSM*.jp*g", "PregSex*.jp*g", "pregbeast*.jp*g", "preggroup*.jp*g", "pregles*.jp*g"};
    int i = 0;
    
    do
    {
        bool to_add = true;
        FileList the_files( imagedir, pic_types[i].c_str() );
        
        for( int k = 0; k < the_files.size(); k++ )
        {
            bool test = false;
            
            if( i == 6 )
            {
                char c = the_files[k].leaf()[4];
                
                for( int j = 0; j < 9; j++ )
                {
                
                    if( c == numeric[j] )
                    {
                        test = true;
                        break;
                    }
                }
                
                if( !test )
                {
                    k = the_files.size();
                    to_add = false;
                }
            }
            
            if( to_add )
            {
                current->m_Images[i].AddImage( the_files[k].full() );
                current->m_Images[i].m_NumImages++;
            }
        }
        
        i++;
        
    }
    while( i < 15 );
    
    // Yes this is just a hack to load animations (my bad ;) - Necro
    std::string pic_types2[] = {"Anal*.ani", "BDSM*.ani", "Sex*.ani", "Beast*.ani", "Group*.ani", "Les*.ani", "Preg*.ani", "Death*.ani", "Profile*.ani", "PregAnal*.ani", "PregBDSM*.ani", "PregSex*.ani", "pregbeast*.ani", "preggroup*.ani", "pregles*.ani"};
    i = 0;
    
    do
    {
        bool to_add = true;
        FileList the_files( imagedir, pic_types2[i].c_str() );
        
        for( int k = 0; k < the_files.size(); k++ )
        {
            bool test = false;
            
            if( i == 6 )
            {
                char c = the_files[k].leaf()[4];
                
                for( int j = 0; j < 9; j++ )
                {
                
                    if( c == numeric[j] )
                    {
                        test = true;
                        break;
                    }
                }
                
                if( !test )
                {
                    k = the_files.size();
                    to_add = false;
                }
            }
            
            if( to_add )
            {
                current->m_Images[i].AddImage( the_files[k].full(), the_files[k].path(), the_files[k].leaf() );
                current->m_Images[i].m_NumImages++;
            }
        }
        
        i++;
        
    }
    while( i < 15 );
    
    if( m_Last )
    {
        m_Last->m_Next = current;
        m_Last = current;
    }
    else
        m_First = m_Last = current;
        
    return current;
}

cImage::cImage()
{
    ;
}

cImage::~cImage()
{
    //g_LogFile.ss() << "cImage::~cImage() [" << this << "]";
    //g_LogFile.ssend();
    
    if( m_Surface && !m_Surface->m_SaveSurface )
    {
        m_Surface.reset();
    }
        
    if( m_AniSurface )
        delete m_AniSurface;
        
    m_AniSurface = nullptr;
    //if(m_Next) delete m_Next;
    m_Next = nullptr;
    
    //g_LogFile.ss() << "cImage::~cImage() finished [" << this << "]";
    //g_LogFile.ssend();
}

} // namespace WhoreMasterRenewal
