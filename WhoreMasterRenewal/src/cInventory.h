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
#ifndef CINVENTORY_H_INCLUDED_1525
#define CINVENTORY_H_INCLUDED_1525
#pragma once

#include "Constants.h"

#include <ostream>
#include <string>
#include <vector>

namespace WhoreMasterRenewal
{

class cInventory;
extern cInventory g_InvManager;

class Girl;

struct sEffect
{
    // MOD docclox
    /*
     *  let's have an enum for possible values of m_Affects
     */
    enum What
    {
        Skill       = 0,
        Stat        = 1,
        Nothing     = 2,
        GirlStatus  = 3,
        Trait       = 4
    };
    
    /*
     *  define an ostream operator so it will pretty print
     *  (more useful for debugging than game play
     *  but why take it out?)
     */
    friend std::ostream& operator << ( std::ostream& os, sEffect::What& w );
    /*
     *  and a function to go the other way
     *  we need this to turn the strings in the xml file
     *  into numbers
     */
    void set_what( std::string s );
    /*
     *  can't make an enum for this since it can represent
     *  different quantites.
     *
     *  The OO approach would be to write some variant classes, I expect
     *  but really? Life's too short...
     */
    
    /*
     *  but I still need strings for the skills, states, traits and so forth
     *
     *  these should be (were until the merge) in Girl. Will be again
     *  as soon as I sort the main mess out...
     */
    const char* girl_status_name( unsigned int id );
    const char* skill_name( unsigned int id );      // WD:  Use definition in Girl::
    const char* stat_name( unsigned int id );       // WD:  Use definition in Girl::
    
    /*
     *  and we need to go the other way,
     *  setting m_EffectID from the strings in the XML file
     *
     *  WD: Change to use definition and code in Girl::
     *      remove duplicated code
     */
    bool set_skill( std::string s );
    bool set_girl_status( std::string s );
    bool set_stat( std::string s );
    
    
    
    What m_Affects = {};
    unsigned char m_EffectID = {0};   // what stat, skill or status effect it affects
    /*
     *  magnitude of the effect.
     *  -10 will subtract 10 from the target stat while equiped
     *  and add 10 when unequiped.
     *
     *  With status effects and traits 1 means add,
     *  0 means take away and 2 means disable
     */
    int m_Amount = {0};
    std::string m_Trait = {"Default m_Trait name"}; // name of the trait it adds
    /*
     *  and a pretty printer for the class as a whole
     *  just a debug thing, really
     */
    // end mod
};

std::ostream& operator << ( std::ostream& os, sEffect& eff );

typedef struct sInventoryItem
{
    std::string m_Name = {"Default sInventoryItem::m_Name"};
    std::string m_Desc = {"Default sInventoryItem::m_Desc"};
    
    /*
     *  item type: let's make an enum
     */
    enum Type
    {
        Ring    = 1,    // worn on fingers (max 8)
        Dress   = 2,    // Worn on body, (max 1)
        Shoes   = 3,    // worn on feet, (max 1)
        Food    = 4,    // Eaten, single use
        Necklace = 5,   // worn on neck, (max 1)
        Weapon  = 6,    // equiped on body, (max 2)
        Makeup  = 7,    // worn on face, single use
        Armor   = 8,    // worn on body over dresses (max 1)
        Misc    = 9,    // random stuff. may cause a constant effect without having to be equiped
        Armband = 10,   // (max 2), worn around arms
        SmWeapon = 11   // (max 1), hidden on body
    };
    Type m_Type = {Type::Misc};
    
    /*
     *  and another for special values
     */
    enum Special
    {
        None        = 0,
        AffectsAll  = 1,
        Temporary   = 2
    };
    Special m_Special = {Special::None};
    
    bool m_Infinite = {false}; // if 1 then this item doesn't run out if stocked in shop inventory
    std::vector<sEffect> m_Effects = {}; // the number of effects this item has
    long m_Cost = {0}; // how much the item is worth?
    
    /*  0 is good, while badness > is evil.
     *  Girls may fight back if badness > 0,
     *  Girls won't normally buy items > 20 on their own
     *      default formula is -5% chance to buy on their own per Badness point (5 Badness = 75% chance)
     *  Girls with low obedience may take off something that is bad for them
     */
    unsigned char m_Badness = {0};
    
    // chance that a girl on break will buy this item if she's looking at it in the shop
    unsigned char m_GirlBuyChance = {0};
    
    /*
     *  0 means common,
     *  1 means 50% chance of appearing in shops,
     *  2 means 25% chance,
     *  3 means 5% chance and
     *  4 means only 15% chance of being found in catacombs,
     *  5 means ONLY given in scripts and
     *  6 means same as 5 but also may be given as a reward for objective
        7 means only 5% chance in catacombs (catacombs05)
        8 means only 1% chance in catacombs (catacombs01)
     */
    enum Rarity
    {
        Common      = 0,
        Shop50      = 1,
        Shop25      = 2,
        Shop05      = 3,
        Catacomb15  = 4,
        ScriptOnly  = 5,
        ScriptOrReward  = 6,
        Catacomb05 = 7,   // MYR: Added 05 and 01 for the really, really valuable things like invulnerability
        Catacomb01 = 8
    };
    Rarity m_Rarity = {Rarity::Common};
    
    void set_rarity( std::string s );
    void set_special( std::string s );
    void set_type( std::string s );
} sInventoryItem;

std::ostream& operator << ( std::ostream& os, sInventoryItem::Special& spec );
std::ostream& operator << ( std::ostream& os, sInventoryItem::Rarity& r );
std::ostream& operator << ( std::ostream& os, sInventoryItem::Type& typ );
std::ostream& operator << ( std::ostream& os, sInventoryItem& it );

int itemcompless( const sInventoryItem* , const sInventoryItem* );
int part( int first, int last, sInventoryItem* curr[] );
void qswap( int, int, sInventoryItem* curr[] );
void qu_sort( int, int, sInventoryItem* curr[] );

class cInventory
{
public:
    cInventory();
    ~cInventory();
    
    cInventory( const cInventory& ) = delete;
    cInventory& operator = ( const cInventory& ) = delete;
    
    void Free();
    
    void LoadItems( std::string filename );
    bool LoadItemsXML( std::string filename );
    void UpdateShop();  // re-randomizes the shops inventory
    sInventoryItem* GetItem( std::string name );
    sInventoryItem* GetShopItem( int num );
    int GetRandomShopItem();
    sInventoryItem* GetRandomItem();
    int CheckShopItem( std::string name );  // checks if a item is in shop inventory, returns -1 if not and the id if it is
    sInventoryItem* BuyShopItem( int num ); // removes and returns the item from the shop
    bool GirlBuyItem( Girl* girl, int ShopItem, int MaxItems, bool AutoEquip ); // girl buys selected item if possible; returns true if bought
    
    void Equip( Girl* girl, int num, bool force );
    void Equip( Girl* girl, sInventoryItem* item, bool force );
    void Unequip( Girl* girl, int num );
    
    void AddItem( sInventoryItem* item );
    void CalculateCost( sInventoryItem* newItem );
    
    int HappinessFromItem( sInventoryItem* item ); // determines how happy a girl will be to receive an item (or how unhappy to lose it)
    
    void GivePlayerAllItems();
    
    bool IsItemEquipable( sInventoryItem* item )
    {
        if( item->m_Type != INVMISC )
            return true;
            
        return false;
    }
    void sort();
    
    bool    equip_limited_item_ok( Girl*, int, bool, int );
    bool    equip_pair_ok( Girl*, int, bool );
    bool    equip_ring_ok( Girl*, int, bool );
    bool    equip_singleton_ok( Girl*, int, bool );
    bool    ok_2_equip( Girl*, int, bool );
    void    remove_trait( Girl*, int, int );
    
    
private:
    std::vector<sInventoryItem*> m_Items = {};   // Master list of items?
    int m_NumShopItems = 0; // number of items in the shop
    sInventoryItem* m_ShopItems[NUM_SHOPITEMS]; // pointers to all items, the shop can only hold 30 random items
};

} // namespace WhoreMasterRenewal

#endif // CINVENTORY_H_INCLUDED_1525
