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
#include "cTariff.h"
#include "cEvents.h"
#include "math.h"
#include "Brothel.h"
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

class GirlPredicate_GRG : public GirlPredicate
{
    bool m_slave;
    bool m_catacomb;
    
public:
    GirlPredicate_GRG( bool slave, bool catacomb )
        : m_slave( slave ),
        m_catacomb( catacomb )
    {
        ;
    }
    
    virtual bool test( Girl* girl )
    {
        return  girl->is_slave() == m_slave
                &&  girl->is_monster() == m_catacomb;
    }
};


// ----- Create / destroy

cGirls::cGirls()
    : m_Names()
{
    m_Names.load( DirPath() << "Resources" << "Data" << "RandomGirlNames.txt" );
}

cGirls::~cGirls()
{
    Free();
    m_ImgListManager.Free();
}

bool cGirls::CheckInvSpace( Girl* girl )
{
    if( girl->m_NumInventory == 40 )
        return false;
    
    return true;
}

void cGirls::SetAntiPreg( Girl* girl, bool useAntiPreg )
{
    girl->m_UseAntiPreg = useAntiPreg;
}

void cGirls::Free()
{
    if( m_Parent )
        delete m_Parent;
        
    m_Parent = nullptr;
    m_Last = nullptr;
    m_NumGirls = 0;
    g_GenGirls = false;
    
    if( m_RandomGirls )
        delete m_RandomGirls;
        
    m_RandomGirls = nullptr;
    m_LastRandomGirls = nullptr;
    m_NumRandomGirls = 0;
    
    m_DefImages = nullptr;
}

// ----- Utility

static char* n_strdup( const char* s )
{
    return strcpy( new char[strlen( s ) + 1], s );
}

Girl* sRandomGirl::lookup = new Girl();  // used to look up stat and skill IDs

// ----- Misc

/*
 * if this returns true, the girl will disobey
 */
bool cGirls::DisobeyCheck( Girl* girl, int action, Brothel* brothel )
{
    int diff;
    int chance_to_obey = 0;     // high value - more likely to obey
    /*
     *  let's start out with the basic rebelliousness
     */
    chance_to_obey = -GetRebelValue( girl, false );
    /*
     *  let's normalise that:
     *  multiply by -1 to make high values more obedient;
     *  add 100 to make it range from 0 to 200
     *  and then divide by 2 to get a conventional percentage value
     */
    //chance_to_obey *= -1;
    chance_to_obey += 100;
    chance_to_obey /= 2;
    
    /*
     *  OK, let's factor in having a matron: normally this is done in GetRebelValue
     *  but matrons have shifts now, so really we want twice the effect for a matron
     *  on each shift as we'd get from just one. Either that, or we need to make this
     *  check shift dependent.
     *
     *  Anyway, the old version added 15 for a matron either shift. Let's add
     *  8 for each shift. Full coverage gets you 16 points
     */
    if( brothel )
    {
        if( brothel->matron_on_shift( SHIFT_DAY ) ) chance_to_obey += 8;
        
        if( brothel->matron_on_shift( SHIFT_NIGHT ) ) chance_to_obey += 8;
    }
    
    /*
     *  This is still confusing - at least it still confuses me
     *  why not normalise the rebellion -100 to 100 value so it runs
     *  0 to 100, and invert it so it's basically an obedience check
     */
    
    switch( action )
    {
    case ACTION_COMBAT:
        /*
         *      I thought I did this before - must have been lost in the merge
         *      somewhere.
         *
         *      anyway, separate combat checks into two - half
         *      for magic and half for combat. If a girl is
         *      an archmage in her spare time, but doesn't know which
         *      end of a rapier to hold - that ought to cancel out
         *
         *      Also, let's make this a sliding scale: 60% is the break-even
         *      point (whores *should* require some training before they'll work
         *      as soldiers) and for every 5 points above or below that
         *      there's a + or -1 modifier
         */
        
#if 1   // WD use best stat as many girls have only one stat high
        
        diff = std::max( girl->combat(), girl->magic() ) - 50;
        diff /= 3;
#else
        diff = girl->combat() - 50;
        diff /= 5;
        chance_to_obey += diff;
        diff = girl->magic() - 50;
        diff /= 5;
#endif
        chance_to_obey += diff;
        break;
        
    case ACTION_SEX:
        /*
         *      Let's do the same thing here
         *
         *      Just noticed that high libido was lowering the chances
         *      of obedience...
         */
        diff = girl->libido() /*- 40*/;  // MYR
        diff /= 5;
        chance_to_obey += diff;
        break;
        
    default:
        break;
    }
    
    /*
     *  add in her enjoyment level
     */
    chance_to_obey += girl->m_Enjoyment[action];
    /*
     *  let's add in some mods for love, fear and hate
     */
    chance_to_obey += girl->pclove() / 10;
    chance_to_obey += girl->pcfear() / 10;
    chance_to_obey -= girl->pchate() / 10;
    /*
     *  Let's add a blanket 30% to all of that
     */
    chance_to_obey += 30;
    /*
     *  let's get a percentage roll
     */
    int roll = g_Dice.d100();
    diff = chance_to_obey - roll;
    bool girl_obeys = ( diff >= 0 );
    
    /*
     *  there's a price to be paid for relying on love or fear
     *
     *  if the only reason she obeys is love it wears away that love
     *  just a little bit. And if she's only doing it out of fear
     */
    if( girl_obeys )
    {
        if( diff < ( girl->pclove() / 10 ) ) girl->pclove( -1 );
        
        if( diff < ( girl->pcfear() / 10 ) ) girl->pcfear( -1 );
    }
    
    /*
     *  do we need any more than this, really?
     *  we can add in some shaping factors if desired
     */
    return g_Dice.percent( 100 - chance_to_obey );
}

void cGirls::CalculateGirlType( Girl* girl )
{
    int BigBoobs = 0;
    int CuteGirl = 0;
    int Dangerous = 0;
    int Cool = 0;
    int Nerd = 0;
    int NonHuman = 0;
    int Lolita = 0;
    int Elegant = 0;
    int Sexy = 0;
    int NiceFigure = 0;
    int NiceArse = 0;
    int SmallBoobs = 0;
    int Freak = 0;
    
    // init the types to 0
    girl->m_FetishTypes = 0;
    
    if( HasTrait( girl, "Big Boobs" ) )
    {
        BigBoobs += 60;
        CuteGirl -= 5;
        Sexy += 10;
        NiceFigure = 5;
        SmallBoobs -= 50;
    }
    
    if( HasTrait( girl, "Abnormally Large Boobs" ) )
    {
        BigBoobs += 60;
        CuteGirl -= 15;
        NonHuman += 5;
        Freak += 20;
        SmallBoobs -= 50;
    }
    
    if( HasTrait( girl, "Small Scars" ) )
    {
        CuteGirl -= 5;
        Dangerous += 5;
        Cool += 2;
        Freak += 2;
    }
    
    if( HasTrait( girl, "Cool Scars" ) )
    {
        CuteGirl -= 10;
        Dangerous += 20;
        Cool += 30;
        Freak += 5;
    }
    
    if( HasTrait( girl, "Horrific Scars" ) )
    {
        CuteGirl -= 15;
        Dangerous += 30;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Cool Person" ) )
    {
        Dangerous += 5;
        Cool += 60;
        Nerd -= 10;
    }
    
    if( HasTrait( girl, "Nerd" ) )
    {
        CuteGirl += 10;
        Dangerous -= 30;
        Cool -= 30;
        Nerd += 60;
        SmallBoobs += 5;
    }
    
    if( HasTrait( girl, "Clumsy" ) )
    {
        CuteGirl += 10;
        Dangerous -= 20;
        Cool -= 10;
        Nerd += 20;
        Freak += 5;
    }
    
    if( HasTrait( girl, "Fast orgasms" ) )
    {
        Cool += 10;
        Sexy += 30;
    }
    
    if( HasTrait( girl, "Slow orgasms" ) )
    {
        CuteGirl -= 5;
        Cool -= 5;
        Elegant += 5;
        Sexy -= 10;
        Freak += 5;
    }
    
    if( HasTrait( girl, "Quick Learner" ) )
    {
        Cool -= 20;
        Nerd += 30;
    }
    
    if( HasTrait( girl, "Slow Learner" ) )
    {
        CuteGirl += 10;
        Cool += 10;
        Nerd -= 20;
    }
    
    if( HasTrait( girl, "Cute" ) )
    {
        CuteGirl += 60;
        Lolita += 20;
        SmallBoobs += 5;
    }
    
    if( HasTrait( girl, "Strong" ) )
    {
        Dangerous += 20;
        Cool += 20;
        Nerd -= 30;
        NiceFigure += 20;
        NiceArse += 20;
    }
    
    if( HasTrait( girl, "Psychic" ) )
    {
        Dangerous += 10;
        Nerd += 10;
        NonHuman += 10;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Strong Magic" ) )
    {
        Dangerous += 20;
        Nerd += 5;
        NonHuman += 5;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Shroud Addict" ) )
    {
        Dangerous += 5;
        Cool += 15;
        Nerd -= 10;
        Elegant -= 20;
        Sexy -= 20;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Fairy Dust Addict" ) )
    {
        Dangerous += 10;
        Cool += 20;
        Nerd -= 15;
        Elegant -= 25;
        Sexy -= 25;
        Freak += 15;
    }
    
    if( HasTrait( girl, "Viras Blood Addict" ) )
    {
        Dangerous += 15;
        Cool += 25;
        Nerd -= 20;
        Elegant -= 30;
        Sexy -= 30;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Aggressive" ) )
    {
        CuteGirl -= 15;
        Dangerous += 20;
        Lolita -= 5;
        Elegant -= 10;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Not Human" ) )
    {
        NonHuman += 60;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Adventurer" ) )
    {
        Dangerous += 20;
        Cool += 10;
        Nerd -= 20;
        Elegant -= 5;
    }
    
    if( HasTrait( girl, "Assassin" ) )
    {
        Dangerous += 25;
        Cool += 15;
        Nerd -= 25;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Lolita" ) )
    {
        BigBoobs -= 30;
        CuteGirl += 30;
        Dangerous -= 5;
        Lolita += 60;
        SmallBoobs += 15;
    }
    
    if( HasTrait( girl, "Nervous" ) )
    {
        CuteGirl += 10;
        Nerd += 15;
    }
    
    if( HasTrait( girl, "Good Kisser" ) )
    {
        Cool += 10;
        Sexy += 20;
    }
    
    if( HasTrait( girl, "Nymphomaniac" ) )
    {
        Sexy += 15;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Elegant" ) )
    {
        Dangerous -= 30;
        Nerd -= 20;
        NonHuman -= 20;
        Elegant += 60;
        Freak -= 30;
    }
    
    if( HasTrait( girl, "Fake orgasm expert" ) )
    {
        Sexy += 5;
    }
    
    if( HasTrait( girl, "Sexy Air" ) )
    {
        Cool += 5;
        Elegant -= 5;
        Sexy += 10;
    }
    
    if( HasTrait( girl, "Great Figure" ) )
    {
        BigBoobs += 10;
        Sexy += 10;
        NiceFigure += 60;
    }
    
    if( HasTrait( girl, "Great Arse" ) )
    {
        Sexy += 10;
        NiceArse = 60;
    }
    
    if( HasTrait( girl, "Small Boobs" ) )
    {
        BigBoobs -= 60;
        CuteGirl += 25;
        Lolita += 15;
        SmallBoobs += 60;
    }
    
    if( HasTrait( girl, "Broken Will" ) )
    {
        Cool -= 40;
        Nerd -= 40;
        Elegant -= 40;
        Sexy -= 40;
        Freak += 40;
    }
    
    if( HasTrait( girl, "Masochist" ) )
    {
        CuteGirl -= 10;
        Nerd -= 10;
        CuteGirl -= 15;
        Dangerous += 10;
        Elegant -= 10;
        Freak += 30;
    }
    
    if( HasTrait( girl, "Sadistic" ) )
    {
        CuteGirl -= 20;
        Dangerous += 15;
        Nerd -= 10;
        Elegant -= 30;
        Sexy -= 10;
        Freak += 30;
    }
    
    if( HasTrait( girl, "Tsundere" ) || HasTrait( girl, "Yandere" ) )
    {
        Dangerous += 5;
        Cool += 5;
        Nerd -= 5;
        Elegant -= 20;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Meek" ) )
    {
        CuteGirl += 15;
        Dangerous -= 30;
        Cool -= 30;
        Nerd += 30;
    }
    
    if( HasTrait( girl, "Manly" ) )
    {
        CuteGirl -= 15;
        Dangerous += 5;
        Elegant -= 20;
        Sexy -= 20;
        NiceFigure -= 20;
        SmallBoobs += 10;
        Freak += 5;
    }
    
    if( HasTrait( girl, "Merciless" ) )
    {
        CuteGirl -= 20;
        Dangerous += 20;
        Nerd -= 10;
        Elegant -= 5;
    }
    
    if( HasTrait( girl, "Fearless" ) )
    {
        Dangerous += 20;
        Cool += 15;
        Nerd -= 10;
        Elegant -= 10;
    }
    
    if( HasTrait( girl, "Iron Will" ) )
    {
        Dangerous += 10;
        Cool += 10;
        Nerd -= 5;
        Elegant -= 10;
    }
    
    if( HasTrait( girl, "Twisted" ) )
    {
        CuteGirl -= 40;
        Dangerous += 30;
        Elegant -= 30;
        Sexy -= 20;
        Freak += 40;
    }
    
    if( HasTrait( girl, "Optimist" ) )
    {
        Elegant += 5;
    }
    
    if( HasTrait( girl, "Pessimist" ) )
    {
        Elegant -= 5;
    }
    
    if( HasTrait( girl, "Dependant" ) )
    {
        CuteGirl += 5;
        Dangerous -= 20;
        Cool -= 5;
        Nerd += 5;
        Elegant -= 20;
        Sexy -= 20;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Sterile" ) )
    {
        Freak += 20;
    }
    
    if( HasTrait( girl, "Fleet of Foot" ) )
    {
        Dangerous += 10;
        Sexy += 20;
    }
    
    if( HasTrait( girl, "Tough" ) )
    {
        CuteGirl -= 5;
        Dangerous += 10;
        Cool += 10;
        Nerd -= 5;
        Elegant -= 5;
    }
    
    if( HasTrait( girl, "One Eye" ) )
    {
        CuteGirl -= 20;
        Cool += 5;
        Dangerous += 10;
        Sexy -= 20;
        NiceFigure -= 10;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Eye Patch" ) )
    {
        CuteGirl -= 5;
        Dangerous += 5;
        Cool += 20;
        Sexy -= 5;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Futanari" ) )
    {
        CuteGirl -= 15;
        NonHuman += 10;
        Freak += 30;
    }
    
    if( HasTrait( girl, "Construct" ) )
    {
        Dangerous += 10;
        NonHuman += 40;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Half-Construct" ) )
    {
        Dangerous += 5;
        NonHuman += 20;
        Freak += 20;
    }
    
    if( HasTrait( girl, "Fragile" ) )
    {
        CuteGirl += 10;
        Nerd += 5;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Mind Fucked" ) )
    {
        CuteGirl -= 60;
        Dangerous -= 60;
        Cool -= 60;
        Nerd -= 60;
        Elegant -= 60;
        Sexy -= 60;
        Freak += 40;
    }
    
    if( HasTrait( girl, "Charismatic" ) )
    {
        Elegant += 30;
        Sexy += 30;
        Freak -= 20;
    }
    
    if( HasTrait( girl, "Charming" ) )
    {
        Elegant += 20;
        Sexy += 20;
        Freak -= 15;
    }
    
    if( HasTrait( girl, "Long Legs" ) )
    {
        Sexy += 20;
        NiceFigure += 20;
    }
    
    if( HasTrait( girl, "Puffy Nipples" ) )
    {
        BigBoobs += 10;
        CuteGirl += 5;
        SmallBoobs += 10;
    }
    
    if( HasTrait( girl, "Perky Nipples" ) )
    {
        BigBoobs += 10;
        CuteGirl += 5;
        SmallBoobs += 10;
    }
    
    if( HasTrait( girl, "Different Colored Eyes" ) )
    {
        NonHuman += 20;
        Freak += 10;
    }
    
    if( HasTrait( girl, "Strange Eyes" ) )
    {
        NonHuman += 20;
        Freak += 15;
    }
    
    if( HasTrait( girl, "Incorporial" ) )
    {
        NonHuman += 60;
        Freak += 40;
    }
    
    if( HasTrait( girl, "MILF" ) )
    {
        Freak += 15;
    }
    
    if( HasTrait( girl, "Cat Girl" ) )
    {
        CuteGirl += 20;
        NonHuman += 60;
        Freak += 5;
    }
    
    if( HasTrait( girl, "Demon" ) )
    {
        Dangerous += 10;
        NonHuman += 60;
        Freak += 5;
    }
    
    if( HasTrait( girl, "Malformed" ) )
    {
        NonHuman += 10;
        Freak += 50;
    }
    
    if( HasTrait( girl, "Retarded" ) )
    {
        NonHuman += 2;
        Freak += 45;
    }
    
    if( BigBoobs > SmallBoobs )
    {
        if( BigBoobs > 50 )
            girl->m_FetishTypes |= ( 1 << FETISH_BIGBOOBS );
    }
    else
    {
        if( SmallBoobs > 50 )
            girl->m_FetishTypes |= ( 1 << FETISH_SMALLBOOBS );
    }
    
    if( CuteGirl > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_CUTEGIRLS );
        
    if( Dangerous > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_DANGEROUSGIRLS );
        
    if( Cool > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_COOLGIRLS );
        
    if( Nerd > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_NERDYGIRLS );
        
    if( NonHuman > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_NONHUMAN );
        
    if( Lolita > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_LOLITA );
        
    if( Elegant > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_ELEGANT );
        
    if( Sexy > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_SEXY );
        
    if( NiceFigure > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_FIGURE );
        
    if( NiceArse > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_ARSE );
        
    if( Freak > 50 )
        girl->m_FetishTypes |= ( 1 << FETISH_FREAKYGIRLS );
}

bool cGirls::CheckGirlType( Girl* girl, int type )
{
    if( type == FETISH_TRYANYTHING )
        return true;
    else if( girl->m_FetishTypes & ( 1 << type ) )
        return true;
        
    return false;
}

void cGirls::CalculateAskPrice( Girl* girl, bool vari )
{
    girl->m_Stats[STAT_ASKPRICE] = 0;
    SetStat( girl, STAT_ASKPRICE, 0 );
    int askPrice = ( int )( ( ( GetStat( girl, STAT_BEAUTY ) + GetStat( girl, STAT_CHARISMA ) ) / 2 ) * 0.6f ); // Initial price
    askPrice += GetStat( girl, STAT_CONFIDENCE ) / 10; // their confidence will make them think they are worth more
    askPrice += GetStat( girl, STAT_INTELLIGENCE ) / 10; // if they are smart they know they can get away with a little more
    askPrice += GetStat( girl, STAT_FAME ) / 2; // And lastly their fame can be quite useful too
    
    if( GetStat( girl, STAT_LEVEL ) > 0 )
        askPrice += GetStat( girl, STAT_LEVEL ) * 10; // MYR: Was * 1
        
    /*
     *  I can't see the sense in reducing a slave's price
     *  if you can't sell slaves
     *
     *  if(girl->m_States&(1<<STATUS_SLAVE))
     *  {
     *      SlaveNeg = (int)((float)askPrice*0.45f);
     *      askPrice -= SlaveNeg;
     *  }
     */
    
    if( vari )
    {
        int minVariance = 0 - ( g_Dice % 10 ) + 1;
        int maxVariance = ( g_Dice % 10 ) + 1;
        int variance = ( ( g_Dice % 10 ) + maxVariance ) - minVariance;
        askPrice += variance;
    }
    
    if( askPrice > 100 )
        askPrice = 100;
        
    UpdateStat( girl, STAT_ASKPRICE, askPrice );
}

sRandomGirl* cGirls::random_girl_at( u_int n )
{
    u_int i;
    sRandomGirl* current = m_RandomGirls;
    
    /*
     *  if we try and get a random girl template
     *  that's numbered higher than the number of
     *  templates... well we're not going to find it.
     *
     *  So let's cut to the chase
     */
    if( n >= m_NumRandomGirls )
    {
        return nullptr;
    }
    
    /*
     *  loop through the linked list n times
     *
     */
    for( i = 0; i < n; i++ )
    {
        current = current->m_Next;
        
        /*
         *      current should only be null at the end
         *      of the chain. Which we shouldn't get to
         *      since we know n < m_NumRandomGirls from
         *      the above.
         *
         *      so if it IS null, we have an integrity error
         *      in the pointer chain.
         *
         *      is it too late to rewrite this using vector?
         */
        if( current == nullptr )
        {
            g_LogFile.ss() << "broken chain in cGirls::random_girl_at" << std::endl;
            g_LogFile.ssend();
            return nullptr;
        }
    }
    
    return current;     // and there we (hopefully) are
}

Girl* cGirls::CreateRandomGirl( int age, bool addToGGirls, bool slave, bool /*undead*/, bool NonHuman, bool childnaped )
{
    cConfig cfg;
    sRandomGirl* current;
    int random_girl_index = 0;
    
    // Mod - removed s - should only need to call it once - docclox
    
    if( m_NumRandomGirls == 0 )
    {
        return nullptr;
    }
    
    /*
     *  pick a number between 0 and m_NumRandomGirls
     */
    random_girl_index = g_Dice % m_NumRandomGirls;
    
    /*
     *  loop until we find a human/non-human template as required
     */
    while( true )
    {
        current = random_girl_at( random_girl_index );
        
        /*
         *      if we couldn't find the girl (which should be impossible_
         *      we pick another number at random and see if that one works
         */
        if( current == nullptr )
        {
            random_girl_index = g_Dice % m_NumRandomGirls;
            continue;
        }
        
        /*
         *      test for humanity - or lack of it as the case may be
         */
        if( NonHuman == ( current->m_Human == 0 ) )
            break;
            
        /*
         *      She's either human when we wanted non-human
         *      or non-human when we wanted human
         *
         *      Either way, try again...
         */
        random_girl_index = g_Dice % m_NumRandomGirls;
    }
    
    if( !current )
    {
        return nullptr;
    }
    
    Girl* newGirl = new Girl();
    newGirl->m_AccLevel = 1;
    newGirl->m_States = 0;
    newGirl->m_NumTraits = 0;
    
    newGirl->m_Desc = current->m_Name;
    
    newGirl->m_Name = new char[current->m_Name.length() + 1]; // name
    strcpy( newGirl->m_Name, current->m_Name.c_str() );
    
    g_LogFile.ss() << "getting money for " << newGirl->m_Name << std::endl;
    g_LogFile.ss() << "template is " << current->m_Name << std::endl;
    g_LogFile.ss() << "min money " << current->m_MinMoney << std::endl;
    g_LogFile.ss() << "max money " << current->m_MaxMoney << std::endl;
    g_LogFile.ssend();
    
    newGirl->m_Money = ( g_Dice % ( current->m_MaxMoney - current->m_MinMoney ) ) + current->m_MinMoney; // money
    
    // skills
    for( u_int i = 0; i < NUM_SKILLS; i++ )
    {
        if( ( int )current->m_MaxSkills[i] == ( int )current->m_MinSkills[i] )
            newGirl->m_Skills[i] = ( int )current->m_MaxSkills[i];
        else if( ( int )current->m_MaxSkills[i] < ( int )current->m_MinSkills[i] )
            newGirl->m_Skills[i] = g_Dice % 101;
        else
            newGirl->m_Skills[i] = ( int )( g_Dice % ( ( int )current->m_MaxSkills[i] - ( int )current->m_MinSkills[i] ) ) + ( int )current->m_MinSkills[i];
    }
    
    // stats
    for( int i = 0; i < NUM_STATS; i++ )
    {
        if( ( int )current->m_MaxStats[i] == ( int )current->m_MinStats[i] )
            newGirl->m_Stats[i] = ( int )current->m_MaxStats[i];
        else if( ( int )current->m_MaxStats[i] < ( int )current->m_MinStats[i] )
            newGirl->m_Stats[i] = g_Dice % 101;
        else
            newGirl->m_Stats[i] = ( int )( g_Dice % ( ( int )current->m_MaxStats[i] - ( int )current->m_MinStats[i] ) ) + ( int )current->m_MinStats[i];
    }
    
    for( int i = 0; i < current->m_NumTraits; i++ ) // add the traits
    {
        int chance = g_Dice % 100 + 1;
        
        if( g_Traits.GetTrait( current->m_Traits[i]->m_Name ) )
        {
            if( chance <= ( int )current->m_TraitChance[i] )
            {
                if( !HasTrait( newGirl, current->m_Traits[i]->m_Name ) )
                    AddTrait( newGirl, current->m_Traits[i]->m_Name );
            }
        }
        else
        {
            std::string message = "cGirls::CreateRandomGirl: ERROR: Trait '";
            message += current->m_Traits[i]->m_Name;
            message += "' from girl template ";
            message += current->m_Name;
            message += " doesn't exist or is spelled incorrectly.";
            g_MessageQue.AddToQue( message, 2 );
        }
    }
    
    if( current->m_Human == 0 )
        AddTrait( newGirl, "Not Human" );
        
    newGirl->m_DayJob = JOB_RESTING;
    newGirl->m_NightJob = JOB_RESTING;
    
    if( !slave )
        newGirl->m_Stats[STAT_HOUSE] = 60;  // 60% is the norm
    else
        newGirl->m_Stats[STAT_HOUSE] = cfg.initial.slave_house_perc();  // 100% is the norm
        
    newGirl->m_Stats[STAT_FAME] = 0;
    
    if( age != 0 )
        newGirl->m_Stats[STAT_AGE] = age;
        
    newGirl->m_Stats[STAT_HEALTH] = 100;
    newGirl->m_Stats[STAT_HAPPINESS] = 100;
    
    if( slave )
    {
        newGirl->m_AccLevel = 0;
        newGirl->m_States |= ( 1 << STATUS_SLAVE );
        newGirl->m_Money = 0;
    }
    
    if( age < 17 )
        AddTrait( newGirl, "Lolita" );
        
    if( g_Dice % 100 <= 3 )
        AddTrait( newGirl, "Shroud Addict" );
        
    if( g_Dice % 100 <= 2 )
        AddTrait( newGirl, "Fairy Dust Addict" );
        
    if( g_Dice % 100 == 1 )
        AddTrait( newGirl, "Viras Blood Addict" );
        
    if( childnaped ) // this girl has been taken against her will so make her rebelious
    {
        newGirl->m_Stats[STAT_SPIRIT] = 100;
        newGirl->m_Stats[STAT_CONFIDENCE] = 100;
        newGirl->m_Stats[STAT_OBEDIENCE] = 0;
        newGirl->m_Stats[STAT_PCHATE] = 50;
    }
    
    newGirl->m_Next = nullptr;
    
    // If the girl is a slave then she has a house percent of 100%
    if( newGirl->m_States & ( 1 << STATUS_SLAVE ) )
    {
        newGirl->m_AccLevel = 0;
        
        if( ( ( int )newGirl->m_Stats[STAT_OBEDIENCE] + 20 ) > 100 )
            newGirl->m_Stats[STAT_OBEDIENCE] = 100;
        else
            newGirl->m_Stats[STAT_OBEDIENCE] += 20;
    }
    
    // Load any girl images if available
    LoadGirlImages( newGirl );
    
    /*      apply trait bonuses
     *      WD: Duplicated stat bonuses of traits as
     *          allready applied with addtrait() calls
     */
    
    //ApplyTraits(newGirl);
    
    /*
     *      WD: remove any rembered traits created
     *          from trait incompatibilities
     */
    RemoveAllRememberedTraits( newGirl );
    
    /*
     *  Now that everything is in there, time to give her a random name
     *
     *  we'll try five times for a unique name
     *  if that fails, we'll give her the last one found
     *  this should be ok - assuming that names don't have to be unique
     *
     *  If they do need to be unique, the game will slow drastically as
     *  the number of girls approaches the limit, and hang once it is surpassed.
     *
     *  So I'm assuming non-unique names are ok
     */
    std::string name;
    
    for( int i = 0; i < 5; i++ )
    {
        name = m_Names.random();
        
        if( NameExists( buffer ) )
        {
            continue;
        }
        
        break;
    }
    
    newGirl->m_Realname = name;
    
    newGirl->m_Virgin = false;
    
    if( newGirl->m_Stats[STAT_AGE] <= 17 )
    {
        newGirl->m_Stats[STAT_AGE] = 17;
        newGirl->m_Virgin = true;
    }
    else if( newGirl->m_Stats[STAT_AGE] == 18 )
    {
        if( g_Dice % 3 == 1 )
            newGirl->m_Virgin = true;
    }
    else if( newGirl->m_Stats[STAT_AGE] <= 25 )
    {
        int avg = 0;
        
        for( u_int i = 0; i < NUM_SKILLS; i++ )
        {
            if( i != SKILL_SERVICE )
                avg += ( int )newGirl->m_Skills[i];
        }
        
        avg = avg / ( NUM_SKILLS - 1 );
        
        if( avg < 30 )
            newGirl->m_Virgin = true;
    }
    
    if( newGirl->m_Stats[STAT_AGE] > 20 && HasTrait( newGirl, "Lolita" ) )
        RemoveTrait( newGirl, "Lolita" );
        
    if( newGirl->m_Stats[STAT_AGE] >= 26 )
    {
        if( g_Dice % 100 <= 20 )
            AddTrait( newGirl, "MILF" );
    }
    
    DirPath dp = DirPath()
                 << "Resources"
                 << "Characters"
                 << newGirl->m_Name
                 << "triggers.xml"
                 ;
    newGirl->m_Triggers.LoadList( dp );
    newGirl->m_Triggers.SetGirlTarget( newGirl );
    
    if( addToGGirls )
        AddGirl( newGirl );
        
    CalculateGirlType( newGirl );
    
    return newGirl;
}

bool cGirls::NameExists( std::string name )
{
    Girl* current = m_Parent;
    
    while( current )
    {
        if( current->m_Realname == name )
            return true;
            
        current = current->m_Next;
    }
    
    if( g_Brothels.NameExists( name ) )
        return true;
        
    for( int i = 0; i < 8; i++ )
    {
        if( MarketSlaveGirls[i] )
        {
            if( MarketSlaveGirls[i]->m_Realname == name )
                return true;
        }
    }
    
    return false;
}

void cGirls::LevelUp( Girl* girl )
{
    std::stringstream ss;
    
    // MYR: Changed from 20.  Nothing appears to be level-dependant (beyond sell price) so
    //      this shouldn't affect anything (I hope. Feel free to revert if this breaks
    //      something.)
    //      Fixed the rest of the fn to be consistent with this change.
    if( GetStat( girl, STAT_LEVEL ) >= 255 ) // Unsigned char's max value is 255
        return;
        
    SetStat( girl, STAT_EXP, 0 );
    
    if( GetStat( girl, STAT_LEVEL ) <= 20 )
        LevelUpStats( girl );
        
    UpdateStat( girl, STAT_LEVEL, 1 );
    
    ss << girl->m_Realname << " levelled up to " << girl->level() << ".";
    
    // add traits
    // MYR: One chance to get a new trait every five levels.
    if( GetStat( girl, STAT_LEVEL ) % 5 == 0 )
    {
        int chance = g_Dice % 100;
        
        if( chance <= 10 && !HasTrait( girl, "Good Kisser" ) )
        {
            AddTrait( girl, "Good Kisser" );
            ss << " She has gained the Good Kisser trait.";
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_WARNING );
        }
        else if( chance <= 20 && !HasTrait( girl, "Nymphomaniac" ) )
        {
            AddTrait( girl, "Nymphomaniac" );
            ss << " She has gained the Nymphomaniac trait.";
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_WARNING );
        }
        else if( chance <= 30 && !HasTrait( girl, "Fake orgasm expert" ) )
        {
            AddTrait( girl, "Fake orgasm expert" );
            ss << " She has gained the Fake Orgasm Expert trait.";
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_WARNING );
        }
        else if( chance <= 40 && !HasTrait( girl, "Sexy Air" ) )
        {
            AddTrait( girl, "Sexy Air" );
            ss << " She has gained the Sexy Air trait.";
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_WARNING );
        }
        else if( chance <= 50 && !HasTrait( girl, "Fleet of Foot" ) )
        {
            AddTrait( girl, "Fleet of Foot" );
            ss << " She has gained the Fleet of Foot trait.";
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_WARNING );
        }
        else if( chance <= 60 && !HasTrait( girl, "Charismatic" ) )
        {
            AddTrait( girl, "Charismatic" );
            ss << " She has gained the Charismatic trait.";
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_WARNING );
        }
        else if( chance <= 70 && !HasTrait( girl, "Charming" ) )
        {
            AddTrait( girl, "Charming" );
            ss << " She has gained the Charming trait.";
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_WARNING );
        }
        
        // A bug appeared to supress this message. Instead of making it global it now appears
        // on the girl's regular message list.
        //g_MessageQue.AddToQue(ss.str(), 0);
    }
    
    girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_SUMMARY );
}

void cGirls::LevelUpStats( Girl* girl )
{
    /*
    int DiceSize = 0;
    
    if(HasTrait(girl,"Quick Learner"))
        DiceSize = 4;
    else if(HasTrait(girl,"Slow Learner"))
        DiceSize = 2;
    else
        DiceSize = 3;
    */
    
    // level up stats (only first 8 advance in levelups)
    for( int i = 0; i < 8; i++ )
        UpdateStat( girl, i, ( g_Dice % 4 ) + 1 );
        
    // level up skills
    for( u_int i = 0; i < NUM_SKILLS; i++ )
        UpdateSkill( girl, i, ( g_Dice % 4 ) + 1 );
}

// ----- Add remove

void cGirls::AddRandomGirl( sRandomGirl* girl )
{
    girl->m_Next = nullptr;
    
    if( m_RandomGirls )
        m_LastRandomGirls->m_Next = girl;
    else
        m_RandomGirls = girl;
        
    m_LastRandomGirls = girl;
    m_NumRandomGirls++;
}

void cGirls::AddGirl( Girl* girl )
{
    girl->m_Prev = nullptr;
    girl->m_Next = nullptr;
    
    if( m_Parent )
    {
        girl->m_Prev = m_Last;
        m_Last->m_Next = girl;
        m_Last = girl;
    }
    else
        m_Last = m_Parent = girl;
        
    m_NumGirls++;
}

void cGirls::RemoveGirl( Girl* girl, bool deleteGirl )
{
    if( m_Parent == nullptr )
        return;
        
    bool match = false;
    
    Girl* currGirl = m_Parent;
    
    while( currGirl )
    {
        if( currGirl == girl )
        {
            match = true;
            break;
        }
        
        currGirl = currGirl->m_Next;
    }
    
    if( match )
    {
        if( deleteGirl )
        {
            if( girl->m_Prev )
                girl->m_Prev->m_Next = girl->m_Next;
                
            if( girl->m_Next )
                girl->m_Next->m_Prev = girl->m_Prev;
                
            if( girl == m_Parent )
                m_Parent = girl->m_Next;
                
            if( girl == m_Last )
                m_Last = girl->m_Prev;
                
            girl->m_Next = nullptr;
            girl->m_Prev = nullptr;
            delete girl;
            girl = nullptr;
        }
        else
        {
            if( girl->m_Prev )
                girl->m_Prev->m_Next = girl->m_Next;
                
            if( girl->m_Next )
                girl->m_Next->m_Prev = girl->m_Prev;
                
            if( girl == m_Parent )
                m_Parent = girl->m_Next;
                
            if( girl == m_Last )
                m_Last = girl->m_Prev;
                
            girl->m_Next = nullptr;
            girl->m_Prev = nullptr;
        }
        
        m_NumGirls--;
    }
}

void cGirls::AddTiredness( Girl* girl )
{
    if( g_Girls.HasTrait( girl, "Incorporial" ) ) // Sanity check
    {
        g_Girls.SetStat( girl, STAT_TIREDNESS, 0 );
        return;
    }
    
    int tiredness = 10;
    
    if( g_Girls.GetStat( girl, STAT_CONSTITUTION ) > 0 )
        tiredness -= ( g_Girls.GetStat( girl, STAT_CONSTITUTION ) ) / 10;
        
    if( tiredness <= 0 )
        tiredness = 0;
        
    g_Girls.UpdateStat( girl, STAT_TIREDNESS, tiredness );
    
    if( g_Girls.GetStat( girl, STAT_TIREDNESS ) == 100 )
    {
        g_Girls.UpdateStat( girl, STAT_HAPPINESS, -1 );
        g_Girls.UpdateStat( girl, STAT_HEALTH, -1 );
    }
}

// ----- Get

int cGirls::GetSlaveGirl( int from )
{
    Girl* current = m_Parent;
    int num = 0;
    int girlnum = 0;
    bool found = false;
    
    // count to the girl at the number from
    //while(current)
    //{
    //  if(num == from)
    //      break;
    //  num++;
    //  current = current->m_Next;
    //}
    
    while( current )
    {
        if( current->m_States & ( 1 << STATUS_SLAVE ) )
        {
            if( num == from )
            {
                found = true;
                break;
            }
            else
                num++;
                
        }
        
        girlnum++;
        current = current->m_Next;
    }
    
    if( found == false )
        return -1;
        
    return girlnum;
}

std::vector<Girl*>  cGirls::get_girls( GirlPredicate* pred )
{
    Girl* girl;
    std::vector<Girl*> v;
    
    for( girl = m_Parent; girl; girl = girl->m_Next )
    {
        if( pred->test( girl ) )
        {
            v.push_back( girl );
        }
    }
    
    return v;
}

std::string cGirls::GetGirlMood( Girl* girl )
{
    std::string ret = "Her Feelings: ";
    int variable = 0;
    
    int HateLove = GetStat( girl, STAT_PCLOVE ) - GetStat( girl, STAT_PCHATE );
    ret += "Feels the player ";
    
    if( HateLove < 0 )
    {
        if( HateLove > -20 )
            ret += "is annoying ";
        else if( HateLove > -40 )
            ret += "isn't nice ";
        else if( HateLove > -60 )
            ret += "is mean ";
        else if( HateLove > -80 )
            ret += "is better off dead ";
        else
            ret += "should die ";
    }
    else
    {
        if( HateLove < 20 )
            ret += "is ok ";
        else if( HateLove < 40 )
            ret += "is easy going ";
        else if( HateLove < 60 )
            ret += "is good ";
        else if( HateLove < 80 )
            ret += "is attractive ";
        else
            ret += "is her true love ";
    }
    
    if( GetStat( girl, STAT_PCFEAR ) > 20 )
    {
        if( HateLove > 0 )
            ret += "but she is also ";
        else
            ret += "and she is ";
            
        if( GetStat( girl, STAT_PCFEAR ) < 40 )
            ret += "afraid of him.";
        else if( GetStat( girl, STAT_PCFEAR ) < 60 )
            ret += "fearful of him.";
        else if( GetStat( girl, STAT_PCFEAR ) < 80 )
            ret += "afraid he will hurt her.";
        else
            ret += "afraid he will kill her.";
    }
    else
        ret += "and he isn't scary.\n";
        
    variable = GetStat( girl, STAT_HAPPINESS );
    ret += "She is ";
    
    if( variable > 90 )
        ret += "happy.\n";
    else if( variable > 80 )
        ret += "joyful.\n";
    else if( variable > 60 )
        ret += "reasonably happy.\n";
    else if( variable > 40 )
        ret += "unhappy.\n";
    else
        ret += "showing signs of depression.\n";
        
    return ret;
}

std::string cGirls::GetMoreDetailsString( Girl* girl )
{
    if( girl == nullptr )
        return std::string( "" );
        
    std::string data = "Fetish Categories: ";
    
    if( CheckGirlType( girl, FETISH_BIGBOOBS ) )
        data += " |Big Boobs| ";
        
    if( CheckGirlType( girl, FETISH_CUTEGIRLS ) )
        data += " |Cute Girl| ";
        
    if( CheckGirlType( girl, FETISH_DANGEROUSGIRLS ) )
        data += " |Dangerous| ";
        
    if( CheckGirlType( girl, FETISH_COOLGIRLS ) )
        data += " |Cool| ";
        
    if( CheckGirlType( girl, FETISH_NERDYGIRLS ) )
        data += " |Nerd| ";
        
    if( CheckGirlType( girl, FETISH_NONHUMAN ) )
        data += " |Non or part human| ";
        
    if( CheckGirlType( girl, FETISH_LOLITA ) )
        data += " |Lolita| ";
        
    if( CheckGirlType( girl, FETISH_ELEGANT ) )
        data += " |Elegant| ";
        
    if( CheckGirlType( girl, FETISH_SEXY ) )
        data += " |Sexy| ";
        
    if( CheckGirlType( girl, FETISH_FIGURE ) )
        data += " |Nice Figure| ";
        
    if( CheckGirlType( girl, FETISH_ARSE ) )
        data += " |Nice Arse| ";
        
    if( CheckGirlType( girl, FETISH_SMALLBOOBS ) )
        data += " |Small Boobs| ";
        
    if( CheckGirlType( girl, FETISH_FREAKYGIRLS ) )
        data += " |Freaky| ";
        
    data += "\n\n";
    
    std::string jobs[] =
    {
        "combat",
        "working as a whore",
        "doing miscellaneous tasks",  // general
        "cleaning",
        "acting as a matron",
        "working in the bar",
        "working in the gambling hall",
        "producing movies",
        "providing security",
        "doing advertising",
        "torturing people",
        "caring for beasts"
    };
    std::string base = "She";
    std::string text;
    unsigned char count = 0;
    
    for( int i = 0; i < NUM_ACTIONTYPES; ++i )
    {
        if( jobs[i] == "" )
            continue;
            
        if( girl->m_Enjoyment[i] < -70 )
        {
            text = " hates ";
        }
        else if( girl->m_Enjoyment[i] < -50 )
        {
            text = " really dislikes ";
        }
        else if( girl->m_Enjoyment[i] < -30 )
        {
            text = " dislikes ";
        }
        else if( girl->m_Enjoyment[i] < -20 )
        {
            text = " doesn't particularly enjoy ";
        }
        else if( girl->m_Enjoyment[i] < 15 )
        {
            //text=" is indifferent to ";
            continue;  // if she's indifferent, why specify it? Let's instead skip it.
        }
        else if( girl->m_Enjoyment[i] < 30 )
        {
            text = " is happy enough with ";
        }
        else if( girl->m_Enjoyment[i] < 50 )
        {
            text = " likes ";
        }
        else if( girl->m_Enjoyment[i] < 70 )
        {
            text = " really enjoys ";
        }
        else
        {
            text = " loves ";
        }
        
        data += base;
        data += text;
        data += jobs[i] + ".\n";
        count++;
    }
    
    if( count > 0 )
        data += "\nShe is indifferent to all other tasks.\n\n";
    else
        data += "At the moment, she is indifferent to all tasks.\n\n";
        
    data += "\nOther Stats\n\n";
    std::stringstream ss;
    ss << "Charisma: " << GetStat( girl, STAT_CHARISMA ) << "\n";
    ss << "Beauty: " << GetStat( girl, STAT_BEAUTY ) << "\n";
    ss << "Libido: " << GetStat( girl, STAT_LIBIDO ) << "\n";
    ss << "Mana: " << GetStat( girl, STAT_MANA ) << "\n";
    ss << "Intelligence: " << GetStat( girl, STAT_INTELLIGENCE ) << "\n";
    ss << "Confidence: " << GetStat( girl, STAT_CONFIDENCE ) << "\n";
    ss << "Obedience: " << GetStat( girl, STAT_OBEDIENCE ) << "\n";
    ss << "Spirit: " << GetStat( girl, STAT_SPIRIT ) << "\n";
    ss << "Agility: " << GetStat( girl, STAT_AGILITY ) << "\n";
    ss << "Fame: " << GetStat( girl, STAT_FAME ) << "\n";
    data += ss.str();
    
    return data;
}

std::string cGirls::GetDetailsString( Girl* girl, bool purchase )
{
    cConfig cfg;
    cTariff tariff;
    std::stringstream ss;
    /*char buffer[100];*/
    
    if( girl == nullptr )
        return std::string( "" );
        
    std::string data = "Looks: ";
    int variable = ( ( GetStat( girl, STAT_BEAUTY ) + GetStat( girl, STAT_CHARISMA ) ) / 2 );
    data += toString( variable );
    data += "%\n";
    
    data += "Age: ";
    
    if( GetStat( girl, STAT_AGE ) == 100 )
    {
        data += "unknown";
    }
    else
    {
        data += toString( GetStat( girl, STAT_AGE ) );
    }
    
    data += "\n";
    
    data += "Level: ";
    data += toString( GetStat( girl, STAT_LEVEL ) );
    data += " | ";
    data += "Exp: ";
    data += toString( GetStat( girl, STAT_EXP ) );
    data += "\n";
    
    if( girl->m_Virgin )
        data += "She is a virgin\n";
        
    data += "Rebelliousness: ";
    data += toString( GetRebelValue( girl, false ) );
    data += "\n";
    
    if( girl->m_States & ( 1 << STATUS_POISONED ) )
        data += "Is poisoned\n";
        
    if( girl->m_States & ( 1 << STATUS_BADLY_POISONED ) )
        data += "Is badly poisoned\n";
        
    int to_go = cfg.pregnancy.weeks_pregnant() - girl->m_WeeksPreg;
    
    if( girl->m_States & ( 1 << STATUS_PREGNANT ) )
    {
        //g_LogFile.ss() << "config.weeks_preg: " << cfg.pregnancy.weeks_pregnant() << std::endl;
        //g_LogFile.ss() << "to go            : " << to_go << std::endl;
        //g_LogFile.ssend();
        
        data += "Is pregnant, due: ";
        data += toString( to_go );
        data += " weeks\n";
    }
    
    if( girl->m_States & ( 1 << STATUS_PREGNANT_BY_PLAYER ) )
    {
        //g_LogFile.ss() << "config.weeks_preg: " << cfg.pregnancy.weeks_pregnant() << std::endl;
        //g_LogFile.ss() << "to go (player's) : " << to_go << std::endl;
        //g_LogFile.ssend();
        
        data += "Is pregnant with your child, due: ";
        data += toString( to_go );
        data += " weeks\n";
    }
    
    if( girl->m_States & ( 1 << STATUS_SLAVE ) )
        data += "Is branded a slave\n";
        
    if( girl->m_States & ( 1 << STATUS_HAS_DAUGHTER ) )
        data += "Has daughter\n";
        
    if( girl->m_States & ( 1 << STATUS_HAS_SON ) )
        data += "Has Son\n";
        
    if( girl->m_States & ( 1 << STATUS_INSEMINATED ) )
    {
        data += "Is inseminated, due: ";
        data += toString( to_go );
        data += " weeks\n";
    }
    
    if( girl->m_PregCooldown != 0 )
    {
        data += "Cannot get pregnant for: ";
        data += toString( girl->m_PregCooldown );
        data += " weeks\n";
    }
    
    if( girl->is_addict() )
        data += "Has addiciton\n";
        
    variable = GetStat( girl, STAT_HAPPINESS );
    data += "Happiness: ";
    data += toString( GetStat( girl, STAT_HAPPINESS ) );
    data += "%\n";
    
    if( !purchase )
    {
        data += "Health: ";
        data += toString( GetStat( girl, STAT_HEALTH ) );
        data += "%\n";
        
        data += "Tiredness: ";
        data += toString( GetStat( girl, STAT_TIREDNESS ) );
        data += "%\n";
        
        data += "Constitution: ";
        data += toString( GetStat( girl, STAT_CONSTITUTION ) );
        data += "%\n";
        
        data += "Accomodation: ";
        
        if( girl->m_AccLevel == 0 )
            data += "Very Poor\n";
            
        if( girl->m_AccLevel == 1 )
            data += "Adequate\n";
            
        if( girl->m_AccLevel == 2 )
            data += "Nice\n";
            
        if( girl->m_AccLevel == 3 )
            data += "Good\n";
            
        if( girl->m_AccLevel == 4 )
            data += "Wonderful\n";
            
        if( girl->m_AccLevel == 5 )
            data += "High Class\n";
            
        if( g_Gangs.GetGangOnMission( MISS_SPYGIRLS ) )
        {
            data += "Gold: ";
            data += toString( girl->m_Money );
            data += "\n";
        }
        else
        {
            data += "Gold: Unknown\n";
        }
    }
    
    
    int cost = int( tariff.slave_price( girl, purchase ) );
    g_LogFile.ss() << "slave "
                   << ( purchase ? "buy" : "sell" )
                   << "price = "
                   << cost
                   ;
    g_LogFile.ssend();
    ss << "Worth: " << cost << " Gold\n" ;
    data += ss.str();
    ss.str( "" );
    
    data += "Avg Pay per customer: ";
    CalculateAskPrice( girl, false );
    data += toString( g_Girls.GetStat( girl, STAT_ASKPRICE ) );
    data += " gold\n";
    
    /*  // shown elsewhere now
        data += "House Percentage: ";
        cost = g_Girls.GetStat(girl, STAT_HOUSE);
        data += toString(cost);
        data += "%\n";
    */
    
    data += "\nSKILLS\n";
    
    data += "Magic Ability: ";
    data += toString( GetSkill( girl, SKILL_MAGIC ) );
    data += "%\n";
    
    data += "Combat Ability: ";
    data += toString( GetSkill( girl, SKILL_COMBAT ) );
    data += "%\n";
    
    data += "Anal Sex: ";
    data += toString( GetSkill( girl, SKILL_ANAL ) );
    data += "%\n";
    
    data += "BDSM Sex: ";
    data += toString( GetSkill( girl, SKILL_BDSM ) );
    data += "%\n";
    
    data += "Normal Sex: ";
    data += toString( GetSkill( girl, SKILL_NORMALSEX ) );
    data += "%\n";
    
    data += "Bestiality Sex: ";
    data += toString( GetSkill( girl, SKILL_BEASTIALITY ) );
    data += "%\n";
    
    data += "Group Sex: ";
    data += toString( GetSkill( girl, SKILL_GROUP ) );
    data += "%\n";
    
    data += "Lesbian Sex: ";
    data += toString( GetSkill( girl, SKILL_LESBIAN ) );
    data += "%\n";
    
    data += "Service Skills: ";
    data += toString( GetSkill( girl, SKILL_SERVICE ) );
    data += "%\n";
    
    data += "Stripping Sex: ";
    data += toString( GetSkill( girl, SKILL_STRIP ) );
    data += "%\n";
    
    return data;
}

Girl* cGirls::GetRandomGirl( bool slave, bool catacomb )
{
    int num_girls = m_NumGirls;
    
    if( ( num_girls == GetNumSlaveGirls() + GetNumCatacombGirls() ) || num_girls == 0 )
    {
        int r = 3;
        
        while( r )
        {
            CreateRandomGirl( 0, true );
            r--;
        }
    }
    
    GirlPredicate_GRG pred( slave, catacomb );
    std::vector<Girl*> girls = get_girls( &pred );
    
    if( girls.size() == 0 )
    {
        return nullptr;
    }
    
    return girls[
               g_Dice.random( girls.size() )
           ];
}

Girl* cGirls::GetGirl( int girl )
{
    int count = 0;
    Girl* current = m_Parent;
    
    if( girl < 0 || ( unsigned int )girl >= m_NumGirls )
        return nullptr;
        
    while( current )
    {
        if( count == girl )
            return current;
            
        count++;
        current = current->m_Next;
    }
    
    return nullptr;
}

int cGirls::GetRebelValue( Girl* girl, bool matron )
{
    /*
     *  WD: Added test to ingnore STAT_HOUSE value
     *  if doing a job that the palyer is paying
     *  only when processing Day or Night Shift
     *
     *  This is to make it so that the jobs that
     *  cost the player support where the hosue take
     *  has no effect has no impact on the chance of
     *  refusal.
     */
    
    if( HasTrait( girl, "Broken Will" ) )
        return -100;
        
    int chanceNo        = 0;
    
    int houseStat       = GetStat( girl, STAT_HOUSE );
    int happyStat       = GetStat( girl, STAT_HAPPINESS );
    bool girlIsSlave    = girl->is_slave();
    
    if( !girlIsSlave )                              // House Take has no effect on slaves
    {
        // WD   House take of gold has no affect on rebellion if
        //      job is paid by player. eg Matron / cleaner
        if( g_Brothels.is_Dayshift_Processing() )
        {
            if( g_Brothels.m_JobManager.is_job_Paid_Player( girl->m_DayJob ) )
            {
                houseStat = 0;
            }
        }
        else if( g_Brothels.is_Nightshift_Processing() )
        {
            if( g_Brothels.m_JobManager.is_job_Paid_Player( girl->m_NightJob ) )
            {
                houseStat = 0;
            }
        }
    }
    
    if( matron ) // a matron will help convince a girl to obey (this always applies in dungeon)
        chanceNo -= 15;
        
    if( HasTrait( girl, "Retarded" ) )
        chanceNo -= 30;
        
    chanceNo -= GetStat( girl, STAT_PCLOVE ) / 5;
    chanceNo += GetStat( girl, STAT_SPIRIT ) / 2;
    chanceNo -= GetStat( girl, STAT_OBEDIENCE ) / 5;
    
    // having a guarding gang will enforce order
    sGang* gang = g_Gangs.GetGangOnMission( MISS_GUARDING );
    
    if( gang )
        chanceNo -= 10;
        
    chanceNo += GetStat( girl, STAT_TIREDNESS ) / 10; // Tired girls less Rebel
    
    if( happyStat < 50 )                            // Unhappy girls increase Rebel
    {
        chanceNo += ( 50 - happyStat ) / 5;
        
        if( happyStat < 10 )                        // WD:  Fixed missing case Happiness < 10
            chanceNo += 10 - happyStat;             // WD:  Rebel increases by an additional point if happy < 10
    }
    else                                            // happy girls are less cranky, less Rebel
        chanceNo -= ( happyStat - 50 ) / 10;
        
    if( girlIsSlave )                               // Slave Girl lowers rebelinous of course
        chanceNo -= 15;                             // House Take has no impact on slaves
        
    else
    {
        chanceNo += 15;                             // Free girls are a little more rebelious
        
        // Free Girls Income
        if( houseStat < 60 )                        // Take less money than normal lower Rebel
            chanceNo -= ( 60 - houseStat ) / 2;
            
        else // if(houseStat > 60)
        {
            chanceNo += ( houseStat - 60 ) / 2;     // Take more money than normal, more Rebel
            
            if( houseStat >= 100 )                  // Take all the money, more Rebel
                chanceNo += 10;
        }
    }
    
    // guarantee certain rebelliousness values for specific traits
    if( HasTrait( girl, "Mind Fucked" ) && chanceNo > -50 )
        return -50;
        
    if( HasTrait( girl, "Dependant" ) && chanceNo > -40 )
        return -40;
        
    if( HasTrait( girl, "Meek" ) && chanceNo > 20 )
        return 20;
        
    if( chanceNo < -100 )                           // Normalise
        chanceNo = -100;
    else if( chanceNo > 100 )
        chanceNo = 100;
        
    return chanceNo;
}

int cGirls::GetNumCatacombGirls()
{
    int number = 0;
    Girl* current = m_Parent;
    
    while( current )
    {
        if( current->m_States & ( 1 << STATUS_CATACOMBS ) )
            number++;
            
        current = current->m_Next;
    }
    
    return number;
}

int cGirls::GetNumSlaveGirls()
{
    int number = 0;
    Girl* current = m_Parent;
    
    while( current )
    {
        if( current->m_States & ( 1 << STATUS_SLAVE ) )
            number++;
            
        current = current->m_Next;
    }
    
    return number;
}

// ----- Stat

int cGirls::GetStat( Girl* girl, int a_stat )
{
    u_int stat = a_stat;
    int value = 0;
    
    if( stat == STAT_AGE ) // Matches SetStat
        return ( int ) girl->m_Stats[stat];
        
    else if( stat == STAT_EXP )
    {
        value = girl->m_Stats[stat];
        
        if( value < 0 )
            value = 0;
        else if( value > 255 )
            value = 255;
            
        return value;
    }
    
    // Health and tiredness require the incorporial sanity check
    else if( stat == STAT_HEALTH )
    {
        value = girl->m_Stats[stat];
        
        if( g_Girls.HasTrait( girl, "Incorporial" ) )
            value = 100;
        else if( value < 0 )
            value = 0;
        else if( value > 100 )
            value = 100;
            
        return value;
    }
    
    else if( stat == STAT_TIREDNESS )
    {
        value = girl->m_Stats[stat];
        
        if( g_Girls.HasTrait( girl, "Incorporial" ) )
            value = 0;
        else if( value < 0 )
            value = 0;
        else if( value > 100 )
            value = 100;
            
        return value;
    }
    
    // Generic calculation
    value = ( ( int )girl->m_Stats[stat] ) + girl->m_StatMods[stat] + girl->m_TempStats[stat];
    
    if( value < 0 )
        value = 0;
    else if( value > 100 )
        value = 100;
        
    return value;
}

void cGirls::SetStat( Girl* girl, int a_stat, int amount )
{
    u_int stat = a_stat;
    int amt = amount;       // Modifying amount directly isn't a good idea
    
    if( stat == STAT_HOUSE || stat == STAT_HAPPINESS )
    {
        if( amt > 100 )
            amt = 100;
        else if( amt < 0 )
            amt = 0;
            
        girl->m_Stats[stat] = amt;
    }
    // Health and tiredness need the incorporial sanity check
    else if( stat == STAT_HEALTH )
    {
        if( g_Girls.HasTrait( girl, "Incorporial" ) )
            amt = 100;
        else if( amt > 100 )
            amt = 100;
        else if( amt < 0 )
            amt = 0;
            
        girl->m_Stats[stat] = amt;
    }
    else if( stat == STAT_TIREDNESS )
    {
        if( g_Girls.HasTrait( girl, "Incorporial" ) )
            amt = 0;
        else if( amt > 100 )
            amt = 100;
        else if( amt < 0 )
            amt = 0;
            
        girl->m_Stats[stat] = amt;
    }
    else if( stat == STAT_EXP )
    {
        if( amt > 255 )
            amt = 255;
        else if( amt < 0 )
            amt = 0;
            
        girl->m_Stats[stat] = amt;
    }
    else if( stat == STAT_AGE )
    {
        /*
         *  WD: Allow age to be SET to 100
         *      for imortals
         */
        
        if( girl->m_Stats[STAT_AGE] > 99 )
            girl->m_Stats[stat] = 100;
        else if( girl->m_Stats[stat] > 80 )
            girl->m_Stats[stat] = 80;
        else if( girl->m_Stats[stat] < 17 )
            girl->m_Stats[stat] = 17;
        else
            girl->m_Stats[stat] = amt;
    }
    else
    {
        if( amt > 100 /*&& stat != STAT_ASKPRICE*/ ) // STAT_ASKPRICE is the price per sex act and is capped at 100
            girl->m_StatMods[stat] = 100;
        else if( amt < -100 )
            girl->m_StatMods[stat] = -100;
        else
            girl->m_StatMods[stat] = amt;
    }
}

void cGirls::UpdateStat( Girl* girl, int a_stat, int amount )
{
    u_int stat = a_stat;
    
    if( stat == STAT_HEALTH || stat == STAT_HAPPINESS )
    {
        if( HasTrait( girl, "Fragile" ) &&  stat != STAT_HAPPINESS )
            amount -= 3;
        else if( HasTrait( girl, "Tough" ) &&  stat != STAT_HAPPINESS )
            amount += 2;
            
        if( HasTrait( girl, "Construct" ) &&  stat != STAT_HAPPINESS )
        {
            if( amount < -4 )
                amount = -4;
            else if( amount > 4 )
                amount = 4;
        }
        
        if( HasTrait( girl, "Incorporial" ) &&  stat != STAT_HAPPINESS )
        {
            //amount = 0;
            girl->m_Stats[STAT_HEALTH] = 100;   // WD: Sanity - Incorporial health should allways be at 100%
            return;
        }
        
        else
        {
            if( amount != 0 )
            {
                int value = girl->m_Stats[stat] + amount;
                
                if( value > 100 )
                    value = 100;
                else if( value < 0 )
                    value = 0;
                    
                girl->m_Stats[stat] = value;
            }
        }
    }
    
    
    else if( stat == STAT_EXP || stat == STAT_LEVEL )
    {
        int value = girl->m_Stats[stat] + amount;
        
        if( stat == STAT_LEVEL )
        {
            if( value > 30 )
                value = 30;
            else if( value < 0 )
                value = 0;
        }
        else
        {
            if( value > 255 )
                value = 255;
            else if( value < 0 )
                value = 0;
        }
        
        girl->m_Stats[stat] = value;
    }
    
    else if( stat == STAT_TIREDNESS )
    {
        if( HasTrait( girl, "Tough" ) )
            amount -= 2;
        else if( HasTrait( girl, "Fragile" ) )
            amount += 3;
            
        if( HasTrait( girl, "Construct" ) )
        {
            if( amount < -4 )
                amount = -4;
            else if( amount > 4 )
                amount = 4;
        }
        
        if( HasTrait( girl, "Incorporial" ) )
        {
//          amount = 0;
            girl->m_Stats[stat] = 0;                                // WD: Sanity - Incorporial Tirdness should allways be at 0%
        }
        
        else
        {
            if( amount != 0 )
            {
                int value = girl->m_Stats[stat] + amount;
                
                if( value > 100 )
                    value = 100;
                else if( value < 0 )
                    value = 0;
                    
                girl->m_Stats[stat] = value;
            }
        }
    }
    
    else if( stat == STAT_AGE )
    {
        if( girl->m_Stats[STAT_AGE] != 100 && amount != 0 )         // WD: Dont change imortal age = 100
        {
            int value = girl->m_Stats[stat] + amount;
            
            //if(value > 80 && value != 100)
            if( value > 80 )
                value = 80;
            else if( value < 17 )
                value = 17;
                
            girl->m_Stats[stat] = value;
        }
    }
    
    else if( stat == STAT_HOUSE )
    {
        int value = girl->m_Stats[stat] + amount;
        
        if( value > 100 )
            value = 100;
        else if( value < 0 )
            value = 0;
            
        girl->m_Stats[stat] = value;
    }
    
    else
    {
        if( amount != 0 )
        {
            if( amount > 0 )
            {
                if( amount + girl->m_StatMods[stat] > 100 && stat != STAT_ASKPRICE )
                    girl->m_StatMods[stat] = 100;
                else
                    girl->m_StatMods[stat] += amount;
            }
            else
            {
                if( amount + girl->m_StatMods[stat] < -100 )
                    girl->m_StatMods[stat] = -100;
                else
                    girl->m_StatMods[stat] += amount;
            }
        }
    }
}

void cGirls::updateTempStats( Girl* girl )
{
    /*
     *   Normalise to zero by 30%
     */
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    for( int i = 0; i < NUM_STATS; i++ )
    {
        if( girl->m_TempStats[i] != 0 )             // normalize towards 0 by 30% each week
        {
            int newStat = ( int )( float( girl->m_TempStats[i] ) * 0.7 );
            
            if( newStat != girl->m_TempStats[i] )
                girl->m_TempStats[i] = newStat;
            else
            {
                // if 30% did nothing, go with 1 instead
                if( girl->m_TempStats[i] > 0 )
                    girl->m_TempStats[i]--;
                else if( girl->m_TempStats[i] < 0 )
                    girl->m_TempStats[i]++;
            }
        }
    }
}

void cGirls::UpdateTempStat( Girl* girl, int stat, int amount )
{
    if( amount >= 0 )
    {
        if( amount + girl->m_TempStats[stat] > INT_MAX )
            girl->m_TempStats[stat] = INT_MAX;
        else
            girl->m_TempStats[stat] += amount;
    }
    else
    {
        if( amount + girl->m_TempStats[stat] < INT_MIN )
            girl->m_TempStats[stat] = INT_MIN;
        else
            girl->m_TempStats[stat] += amount;
    }
}

// ----- Skill

int cGirls::GetSkill( Girl* girl, int skill )
{
    int value = ( ( int )girl->m_Skills[skill] ) + girl->m_SkillMods[skill] + girl->m_TempSkills[skill];
    
    if( value < 0 )
        value = 0;
    else if( value > 100 )
        value = 100;
        
    return value;
}

void cGirls::SetSkill( Girl* girl, int skill, int amount )
{
    girl->m_SkillMods[skill] = amount;
}

int cGirls::GetSkillWorth( Girl* girl )
{
    int num = 0;
    
    for( u_int i = 0; i < NUM_SKILLS; i++ )
        num += ( int )girl->m_Skills[i];
        
    return num;
}

void cGirls::UpdateSkill( Girl* girl, int skill, int amount )
{
    if( amount >= 0 )
    {
        if( amount + girl->m_SkillMods[skill] > INT_MAX )
            girl->m_SkillMods[skill] = INT_MAX;
        else
            girl->m_SkillMods[skill] += amount;
    }
    else
    {
        if( amount + girl->m_SkillMods[skill] < INT_MIN )
            girl->m_SkillMods[skill] = INT_MIN;
        else
            girl->m_SkillMods[skill] += amount;
    }
}

void cGirls::updateTempSkills( Girl* girl )
{
    /*
     *   Normalise to zero by 30%
     */
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    for( u_int i = 0; i < NUM_SKILLS; i++ )
    {
        if( girl->m_TempSkills[i] != 0 )
        {
            // normalize towards 0 by 30% each week
            int newSkill = ( int )( float( girl->m_TempSkills[i] ) * 0.7 );
            
            if( newSkill != girl->m_TempSkills[i] )
                girl->m_TempSkills[i] = newSkill;
            else
            {
                // if 30% did nothing, go with 1 instead
                if( girl->m_TempSkills[i] > 0 )
                    girl->m_TempSkills[i]--;
                else if( girl->m_TempSkills[i] < 0 )
                    girl->m_TempSkills[i]++;
            }
        }
    }
}

void cGirls::UpdateTempSkill( Girl* girl, int skill, int amount )
{
    if( amount >= 0 )
    {
        if( amount + girl->m_TempSkills[skill] > INT_MAX )
            girl->m_TempSkills[skill] = INT_MAX;
        else
            girl->m_TempSkills[skill] += amount;
    }
    else
    {
        if( amount + girl->m_TempSkills[skill] < INT_MIN )
            girl->m_TempSkills[skill] = INT_MIN;
        else
            girl->m_TempSkills[skill] += amount;
    }
}

// ----- Load save

// This load

void cGirls::LoadGirlLegacy( Girl* current, std::ifstream& ifs )
{
    cConfig cfg;
    int temp = 0;
    std::stringstream ss;
    char buffer[4000];
    
    // load the name
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs.getline( buffer, sizeof( buffer ), '\n' );  // get the name
    current->m_Name = new char[strlen( buffer ) + 1];
    strcpy( current->m_Name, buffer );
    
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs.getline( buffer, sizeof( buffer ), '\n' );
    current->m_Realname = buffer;
    
    if( cfg.debug.log_girls() )
    {
        ss << "Loading girl: '" << current->m_Realname;
        g_LogFile.write( ss.str() );
    }
    
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs.getline( buffer, sizeof( buffer ), '\n' );  // get the description
    current->m_Desc = buffer;
    
    // load the amount of days they are unhappy in a row
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_DaysUnhappy = temp;
    
    // Load their traits
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_NumTraits = temp;
    
    if( current->m_NumTraits > MAXNUM_TRAITS )
        g_LogFile.write( "--- ERROR - Loaded more traits then girls can have??" );
        
    for( int i = 0; i < current->m_NumTraits; i++ ) // load the trait names
    {
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs.getline( buffer, sizeof( buffer ), '\n' );
        current->m_Traits[i] = g_Traits.GetTrait( buffer );
        
        // get the temp trait variable
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs >> temp;
        current->m_TempTrait[i] = temp;
    }
    
    // Load their remembered traits
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_NumRememTraits = temp;
    
    if( current->m_NumRememTraits > MAXNUM_TRAITS * 2 )
        g_LogFile.write( "--- ERROR - Loaded more remembered traits then girls can have??" );
        
    for( int i = 0; i < current->m_NumRememTraits; i++ ) // load the trait names
    {
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs.getline( buffer, sizeof( buffer ), '\n' );
        current->m_RememTraits[i] = g_Traits.GetTrait( buffer );
    }
    
    // Load inventory items
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp; // # of items they have
    current->m_NumInventory = temp;
    sInventoryItem* tempitem;
    int total_num = 0;
    
    for( int i = 0; i < current->m_NumInventory; i++ ) // save in the items
    {
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs.getline( buffer, sizeof( buffer ), '\n' );
        tempitem = g_InvManager.GetItem( buffer );
        
        if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
        
        ifs >> temp; // Equipped or not (0 = no)
        
        if( tempitem )
        {
            current->m_EquipedItems[total_num] = temp;  // Is the item equipped?
            current->m_Inventory[total_num] = tempitem;
            
            total_num++;
        }
    }
    
    // load their states
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> current->m_States;
    
    // load their stats
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    for( int i = 0; i < NUM_STATS; i++ )
    {
        temp = 0;
        ifs >> temp >> current->m_StatMods[i] >> current->m_TempStats[i];
        current->m_Stats[i] = temp;
    }
    
    // load their skills
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    for( u_int i = 0; i < NUM_SKILLS; i++ )
    {
        temp = 0;
        ifs >> temp >> current->m_SkillMods[i] >> current->m_TempSkills[i];
        current->m_Skills[i] = temp;
    }
    
    // load virginity
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    
    if( temp == 1 )
        current->m_Virgin = true;
    else
        current->m_Virgin = false;
        
    // load using antipreg
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    
    if( temp == 1 )
        current->m_UseAntiPreg = true;
    else
        current->m_UseAntiPreg = false;
        
    // load withdrawals
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_Withdrawals = temp;
    
    // load money
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> current->m_Money;
    
    // load acom level
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_AccLevel = temp;
    
    // load day/night jobs
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_DayJob = temp;
    ifs >> temp;
    current->m_NightJob = temp;
    
    // load prev day/night jobs
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_PrevDayJob = temp;
    ifs >> temp;
    current->m_PrevNightJob = temp;
    
    // load runnayway value
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_RunAway = temp;
    
    // load spotted
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_Spotted = temp;
    
    // load weeks past, birth day, and pregant time
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> current->m_WeeksPast >> current->m_BDay;
    ifs >> temp;
    current->m_WeeksPreg = temp;
    
    // load number of customers slept with
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> current->m_NumCusts;
    
    // load girl flags
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    for( int i = 0; i < NUM_GIRLFLAGS; i++ )
    {
        temp = 0;
        ifs >> temp;
        current->m_Flags[i] = temp;
    }
    
    // load their torture value
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    
    if( temp == 1 )
        current->m_Tort = true;
    else
        current->m_Tort = false;
        
    // Load their children
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    current->m_PregCooldown = temp;   // load preg cooldown
    
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    // load number of children
    int temp2 = temp;
    
    if( temp2 > 0 )
    {
        for( int i = 0; i < temp2; i++ )
        {
            sChild* child = new sChild();
            int a, b, c, d;
            
            if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
            
            ifs >> a >> b >> c >> d;
            
            child->m_Age = a;
            child->m_IsPlayers = b;
            child->m_Sex = sChild::Gender( c );
            child->m_Unborn = d;
            
            // load their stats
            if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
            
            for( int j = 0; j < NUM_STATS; j++ )
            {
                temp = 0;
                ifs >> temp;
                child->m_Stats[j] = temp;
            }
            
            // load their skills
            if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
            
            for( u_int j = 0; j < NUM_SKILLS; j++ )
            {
                temp = 0;
                ifs >> temp;
                child->m_Skills[j] = temp;
            }
            
            current->m_Children.add_child( child );
        }
    }
    
    // load enjoyment values
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    for( int i = 0; i < NUM_ACTIONTYPES; i++ )
        ifs >> current->m_Enjoyment[i];
        
    // load their triggers
    current->m_Triggers.LoadTriggersLegacy( ifs );
    current->m_Triggers.SetGirlTarget( current );
    
    // load their images
    LoadGirlImages( current );
    
    if( current->m_Stats[STAT_AGE] < 17 )
        current->m_Stats[STAT_AGE] = 17;
        
    CalculateGirlType( current );
}

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

void cGirls::LoadRandomGirl( std::string filename )
{
    /*
     *  before we go any further: files that end in "x" are
     *  in XML format. Get the last char of the filename.
     */
    char c = filename.at( filename.length() - 1 );
    
    /*
     *  now decide how we want to really load the file
     */
    if( c == 'x' )
    {
        g_LogFile.ss() << "loading " << filename << " as XML" << std::endl;
        g_LogFile.ssend();
        LoadRandomGirlXML( filename );
    }
    else
    {
        g_LogFile.ss() << "loading " << filename << " as Legacy" << std::endl;
        g_LogFile.ssend();
        LoadRandomGirlLegacy( filename );
    }
}

void cGirls::LoadRandomGirlXML( std::string filename )
{
    TiXmlDocument doc( filename );
    
    if( !doc.LoadFile() )
    {
        g_LogFile.ss() << "can't load random XML girls " << filename << std::endl;
        g_LogFile.ss()  << "Error: line " << doc.ErrorRow() << ", col " << doc.ErrorCol()
                        << ": " << doc.ErrorDesc() << std::endl;
        g_LogFile.ssend();
        return;
    }
    
    /*
     *  get the docuement root
     */
    TiXmlElement* el, *root_el = doc.RootElement();
    
    /*
     *  loop over the elements attached to the root
     */
    for( el = root_el->FirstChildElement(); el; el = el->NextSiblingElement() )
    {
        sRandomGirl* girl = new sRandomGirl;
        /*
         *      walk the XML DOM to get the girl data
         */
        girl->load_from_xml( el );
        /*
         *      add the girl to the list
         */
        AddRandomGirl( girl );
    }
}

void cGirls::LoadRandomGirlLegacy( std::string filename )
{
    std::ifstream in;
    in.open( filename.c_str() );
    char buffer[500];
    sRandomGirl* newGirl = nullptr;
    int tempData;
    
    while( in.good() )
    {
        newGirl = new sRandomGirl();
        
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in.getline( buffer, sizeof( buffer ), '\n' );   // get the name
        newGirl->m_Name = buffer;
        
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in.getline( buffer, sizeof( buffer ), '\n' );   // get the description
        newGirl->m_Desc = buffer;
        
        // Get their min stats
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        for( int i = 0; i < NUM_STATS; i++ )
        {
            in >> tempData;
            newGirl->m_MinStats[i] = tempData;
        }
        
        // Get their max stats
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        for( int i = 0; i < NUM_STATS; i++ )
        {
            in >> tempData;
            newGirl->m_MaxStats[i] = tempData;
        }
        
        // get their min skills
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        for( u_int i = 0; i < NUM_SKILLS; i++ )
        {
            in >> tempData;
            newGirl->m_MinSkills[i] = tempData;
        }
        
        // get their max skills
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        for( u_int i = 0; i < NUM_SKILLS; i++ )
        {
            in >> tempData;
            newGirl->m_MaxSkills[i] = tempData;
        }
        
        // Get their min starting cash
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        newGirl->m_MinMoney = tempData;
        
        // Get their max starting cash
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        newGirl->m_MaxMoney = tempData;
        
        // Get if they are human
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        newGirl->m_Human = !( unsigned char )tempData;
        
        // Get if they are found in the catacombs
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        newGirl->m_Catacomb = ( unsigned char )tempData;
        
        // Get how many traits they can have
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        newGirl->m_NumTraits = tempData;
        
        // clear out the rest of the newline
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        if( in.peek() == '\r' ) in.ignore( 1, '\r' );
        
        /*
         *      load in the traits and percentages
         */
        for( int i = 0; i < newGirl->m_NumTraits; i++ ) // load in the traits and percentages
        {
            if( in.peek() == '\n' ) in.ignore( 1, '\n' );
            
            in.getline( buffer, sizeof( buffer ), '\n' );
            
            if( g_Traits.GetTrait( buffer ) == nullptr ) // test a trait exists
            {
                std::string message = "ERROR: Trait ";
                message += buffer;
                message += " from girl template ";
                message += newGirl->m_Name;
                message += " doesn't exist or is spelled incorrectly.";
                g_MessageQue.AddToQue( message, 2 );
            }
            else
            {
                newGirl->m_Traits[i] = g_Traits.GetTrait( buffer ); // get the trait name
                
                if( in.peek() == '\n' ) in.ignore( 1, '\n' );
                
                in >> tempData;
                newGirl->m_TraitChance[i] = ( unsigned char )tempData;  // get the percent chance of getting the trait
                
                if( in.peek() == '\n' ) in.ignore( 1, '\n' );
                
                if( in.peek() == '\r' ) in.ignore( 1, '\r' );
            }
        }
        
        AddRandomGirl( newGirl );
    }
    
    in.close();
}

void cGirls::LoadGirlsDecider( std::string filename )
{
    /*
     *  before we go any further: files that end in "x" are
     *  in XML format. Get the last char of the filename.
     */
    char c = filename.at( filename.length() - 1 );
    
    /*
     *  now decide how we want to really load the file
     */
    if( c == 'x' )
    {
        g_LogFile.ss() << "loading " << filename << " as XML" << std::endl;
        g_LogFile.ssend();
        LoadGirlsXML( filename );
    }
    else
    {
        g_LogFile.ss() << "loading " << filename << " as legacy" << std::endl;
        g_LogFile.ssend();
        LoadGirlsLegacy( filename );
    }
}

void cGirls::LoadGirlsXML( std::string filename )
{
    cConfig cfg;
    
    if( cfg.debug.log_girls() )
    {
        g_LogFile.ss() << "loading " << filename;
        g_LogFile.ssend();
    }
    
    TiXmlDocument doc( filename );
    
    if( !doc.LoadFile() )
    {
        g_LogFile.ss() << "can't load XML girls " << filename << std::endl;
        g_LogFile.ss()  << "Error: line " << doc.ErrorRow() << ", col " << doc.ErrorCol()
                        << ": " << doc.ErrorDesc() << std::endl;
        g_LogFile.ssend();
        return;
    }
    
    /*
     *  get the docuement root
     */
    TiXmlElement* el, *root_el = doc.RootElement();
    
    /*
     *  loop over the elements attached to the root
     */
    for( el = root_el->FirstChildElement(); el; el = el->NextSiblingElement() )
    {
        Girl* girl = new Girl;
        /*
         *      walk the XML DOM to get the girl data
         */
        girl->load_from_xml( el );
        
        if( cfg.debug.log_girls() )
        {
            g_LogFile.ss() << *girl << std::endl;
            g_LogFile.ssend();
        }
        
        /*
         *      Mod: need to do this if you do not want a crash
         */
        g_Girls.LoadGirlImages( girl );
        /*
         *      make sure all the trait effects are applied
         */
        ApplyTraits( girl );
        /*
         *      WD: For new girlsr emove any remembered traits from
         *          trait incompatibilities
         */
        RemoveAllRememberedTraits( girl );
        /*
         *      load triggers if the girl has any
         */
        DirPath dp = DirPath()
                     << "Resources"
                     << "Characters"
                     << girl->m_Name
                     << "triggers.xml"
                     ;
        girl->m_Triggers.LoadList( dp );
        girl->m_Triggers.SetGirlTarget( girl );
        /*
         *      add the girl to the list
         */
        AddGirl( girl );
        /*
         *      not sure what this does, but the legacy version does it
         *      WD: Fetish list for customer happiniess caculation on the
         *          more button of the girl details screen
         */
        CalculateGirlType( girl );
    }
}

void cGirls::LoadGirlsLegacy( std::string filename )
{
    std::ifstream in;
    in.open( filename.c_str() );
    char buffer[500];
    Girl* newGirl = nullptr;
    int tempData;
    cConfig cfg;
    
    while( in.good() )
    {
        newGirl = new Girl;
        
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in.getline( buffer, sizeof( buffer ), '\n' );   // get the name
        newGirl->m_Name = new char[strlen( buffer ) + 1];
        strcpy( newGirl->m_Name, buffer );
        
        newGirl->m_Realname = buffer;
        
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in.getline( buffer, sizeof( buffer ), '\n' );   // get the description
        newGirl->m_Desc = buffer;
        
        // Get how many traits they have
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        in.getline( buffer, sizeof( buffer ), '\n' );   // get the description
        newGirl->m_NumTraits = tempData;
        
        for( int i = 0; i < tempData; i++ ) // load in the traits
        {
            if( in.peek() == '\n' ) in.ignore( 1, '\n' );
            
            if( in.peek() == '\r' ) in.ignore( 1, '\r' );
            
            in.getline( buffer, sizeof( buffer ), '\n' );
            
            if( g_Traits.GetTrait( buffer ) == nullptr ) // test a trait exists
            {
                std::string message = "ERROR in cGirls::LoadGirls: Trait '";
                message += buffer;
                message += "' from girl template ";
                message += newGirl->m_Name;
                message += " doesn't exist or is spelled incorrectly.";
                g_MessageQue.AddToQue( message, 2 );
            }
            else
                newGirl->m_Traits[i] = g_Traits.GetTrait( buffer );
        }
        
        // Get how many inventory items they have
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        newGirl->m_NumInventory = tempData;
        
        for( int i = 0; i < tempData; i++ ) // load in the items
        {
            if( in.peek() == '\n' ) in.ignore( 1, '\n' );
            
            in.getline( buffer, sizeof( buffer ), '\n' );
            //newGirl->m_Inventory[i] = g_InventoryItems.GetItem(buffer);
        }
        
        // Get their stats
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        for( int i = 0; i < NUM_STATS; i++ )
        {
            in >> tempData;
            newGirl->m_Stats[i] = tempData;
        }
        
        // get their skills
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        for( u_int i = 0; i < NUM_SKILLS; i++ )
        {
            in >> tempData;
            newGirl->m_Skills[i] = tempData;
        }
        
        // Get their starting cash
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        newGirl->m_Money = tempData;
        
        // Are they a slave (the only default state)
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in >> tempData;
        
        if( tempData == 1 )
        {
            newGirl->m_AccLevel = 0;
            newGirl->m_States |= ( 1 << STATUS_SLAVE );
            newGirl->m_Stats[STAT_HOUSE] =  cfg.initial.slave_house_perc();
        }
        else if( tempData == 2 )
        {
            newGirl->m_AccLevel = 0;
            newGirl->m_Stats[STAT_HOUSE] = cfg.initial.slave_house_perc();
            newGirl->m_States |= ( 1 << STATUS_CATACOMBS );
        }
        else    // default stats
        {
            newGirl->m_AccLevel = 1;
            newGirl->m_States = 0;
            newGirl->m_Stats[STAT_HOUSE] = 60;
        }
        
        LoadGirlImages( newGirl );
        ApplyTraits( newGirl );
        
        /*
         *      WD: remove any rembered traits created
         *          from trait incompatibilities
         */
        RemoveAllRememberedTraits( newGirl );
        
        DirPath dp = DirPath()
                     << "Resources"
                     << "Characters"
                     << newGirl->m_Name
                     << "triggers.txt"
                     ;
        newGirl->m_Triggers.LoadList( dp );
        newGirl->m_Triggers.SetGirlTarget( newGirl );
        
        if( newGirl->m_Stats[STAT_AGE] <= 16 )
            newGirl->m_Virgin = true;
        else if( newGirl->m_Stats[STAT_AGE] <= 18 )
        {
            if( g_Dice % 3 == 1 )
                newGirl->m_Virgin = true;
        }
        else
        {
            int avg = 0;
            
            for( u_int i = 0; i < NUM_SKILLS; i++ )
            {
                if( i != SKILL_SERVICE )
                    avg += ( int )newGirl->m_Skills[i];
            }
            
            avg = avg / ( NUM_SKILLS - 1 );
            
            if( ( g_Dice % 100 ) + 1 > avg )
            {
                newGirl->m_Virgin = true;
            }
        }
        
        if( newGirl->m_Stats[STAT_AGE] < 17 )
            newGirl->m_Stats[STAT_AGE] = 17;
            
        AddGirl( newGirl );
        CalculateGirlType( newGirl );
        newGirl = nullptr;
    }
    
    in.close();
}

bool cGirls::LoadGirlsXML( TiXmlHandle hGirls )
{
    //cannot call free, it would free stuff we want to keep
    m_Parent = nullptr;
    m_Last = nullptr;
    m_NumGirls = 0;
    
    TiXmlElement* pGirls = hGirls.ToElement();
    
    if( pGirls == nullptr )
    {
        return false;
    }
    
    // load the number of girls
    Girl* current = nullptr;
    
    for( TiXmlElement* pGirl = pGirls->FirstChildElement( "Girl" );
            pGirl != nullptr;
            pGirl = pGirl->NextSiblingElement( "Girl" ) ) // load each girl and add her
    {
        current = new Girl();
        bool success = current->LoadGirlXML( TiXmlHandle( pGirl ) );
        
        if( success == true )
        {
            AddGirl( current );
        }
        else
        {
            delete current;
            continue;
        }
    }
    
    return true;
}

void cGirls::LoadGirlsLegacy( std::ifstream& ifs )
{
    Girl* current = nullptr;
    int temp;
    
#if 0  // should already be cleared before now, and doing again here wipes out any newly loaded girl packs (from LoadMasterFile)
    
    if( m_Parent )
        delete m_Parent;
        
    m_Parent = m_Last = 0;
    m_NumGirls = 0;
#endif
    
    // load the number of girls
    if( ifs.peek() == '\n' ) ifs.ignore( 1, '\n' );
    
    ifs >> temp;
    
    for( int i = 0; i < temp; i++ ) // load each girl and add her
    {
        current = new Girl();
        LoadGirlLegacy( current, ifs );
        AddGirl( current );
        current = nullptr;
    }
}

TiXmlElement* cGirls::SaveGirlsXML( TiXmlElement* pRoot )
{
    TiXmlElement* pGirls = new TiXmlElement( "Girls" );
    pRoot->LinkEndChild( pGirls );
    Girl* current = m_Parent;
    
    // save the number of girls
    while( current )
    {
        current->SaveGirlXML( pGirls );
        current = current->m_Next;
    }
    
    return pGirls;
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

// ----- Equipment & inventory

int cGirls::HasItem( Girl* girl, std::string name )
{
    for( int i = 0; i < 40; i++ )
    {
        if( girl->m_Inventory[i] )
        {
            if( girl->m_Inventory[i]->m_Name == name )
                return i;
        }
    }
    
    return -1;
}

void cGirls::EquipCombat( Girl* girl )
{
    // girl makes sure best armor and weapons are equipped, ready for combat
    cConfig cfg;
    
    if( !cfg.items.auto_combat_equip() ) // this feature disabled in config?
        return;
        
    // if she's retarded, she might refuse or forget
    int refusal = 0;
    
    if( girl->has_trait( "Retarded" ) )
        refusal += 30;
        
    if( g_Dice.percent( refusal ) )
        return;
        
    int Armor = -1, Weap1 = -1, Weap2 = -1;
    
    for( int i = 0; i < 40; i++ )
    {
        if( girl->m_Inventory[i] != nullptr )
        {
            if( girl->m_Inventory[i]->m_Type == INVWEAPON )
            {
                g_InvManager.Unequip( girl, i );
                
                if( Weap1 == -1 )
                    Weap1 = i;
                else if( Weap2 == -1 )
                    Weap2 = i;
                else if( girl->m_Inventory[i]->m_Cost > girl->m_Inventory[Weap1]->m_Cost )
                {
                    Weap2 = Weap1;
                    Weap1 = i;
                }
                else if( girl->m_Inventory[i]->m_Cost > girl->m_Inventory[Weap2]->m_Cost )
                    Weap2 = i;
            }
            
            if( girl->m_Inventory[i]->m_Type == INVARMOR )
            {
                g_InvManager.Unequip( girl, i );
                
                if( Armor == -1 )
                    Armor = i;
                else if( girl->m_Inventory[i]->m_Cost > girl->m_Inventory[Armor]->m_Cost )
                    Armor = i;
            }
        }
    }
    
    if( Armor > -1 )
        g_InvManager.Equip( girl, Armor, false );
        
    if( Weap1 > -1 )
        g_InvManager.Equip( girl, Weap1, false );
        
    if( Weap2 > -1 )
        g_InvManager.Equip( girl, Weap2, false );
}

void cGirls::UnequipCombat( Girl* girl )
{
    // girl unequips armor and weapons, ready for brothel work or other non-aggressive jobs
    cConfig cfg;
    
    if( !cfg.items.auto_combat_equip() ) // this feature disabled in config?
        return;
        
    // if she's a really rough or crazy bitch, she might just keep combat gear equipped
    int refusal = 0;
    
    if( girl->has_trait( "Aggressive" ) )
        refusal += 30;
        
    if( girl->has_trait( "Yandere" ) )
        refusal += 30;
        
    if( girl->has_trait( "Twisted" ) )
        refusal += 30;
        
    if( girl->has_trait( "Retarded" ) )
        refusal += 30;
        
    if( g_Dice.percent( refusal ) )
        return;
        
    for( int i = 0; i < 40; i++ )
    {
        if( girl->m_Inventory[i] != nullptr )
        {
            sInventoryItem* curItem = girl->m_Inventory[i];
            
            if( curItem->m_Type == INVWEAPON )
                g_InvManager.Unequip( girl, i );
            else if( curItem->m_Type == INVARMOR )
            {
#if 0  // code to only unequip armor if it has bad effects... unfinished and disabled for the time being; not sure it should be used anyway
                bool badEffects = false;
                
                for( u_int j = 0; j < curItem->m_Effects.size(); j++ )
                {
                    sEffect* curEffect = &curItem->m_Effects[j];
                    
                    if( curEffect->m_Affects == sEffect::Stat && curEffect->m_Amount < 0 )
                    {
                    }
                    else if( curEffect->m_Affects == sEffect::Skill && curEffect->m_Amount < 0 )
                    {
                    }
                }
                
                if( badEffects )
#endif
                    g_InvManager.Unequip( girl, i );
            }
        }
    }
}

void cGirls::UseItems( Girl* girl )
{
    bool withdraw = false;
    
    // uses drugs first
    if( HasTrait( girl, "Viras Blood Addict" ) )
    {
        int temp = HasItem( girl, "Vira Blood" );
        
        if( temp == -1 ) // withdrawals for a week
        {
            if( girl->m_Withdrawals >= 30 )
                RemoveTrait( girl, "Viras Blood Addict" );
            else
            {
                UpdateStat( girl, STAT_HAPPINESS, -30 );
                UpdateStat( girl, STAT_OBEDIENCE, -30 );
                UpdateStat( girl, STAT_HEALTH, -4 );
                
                if( !withdraw )
                {
                    girl->m_Withdrawals++;
                    withdraw = true;
                }
            }
        }
        else
        {
            UpdateStat( girl, STAT_HAPPINESS, 10 );
            UpdateTempStat( girl, STAT_LIBIDO, 10 );
            g_InvManager.Equip( girl, temp, false );
        }
    }
    
    if( HasTrait( girl, "Fairy Dust Addict" ) )
    {
        int temp = HasItem( girl, "Fairy Dust" );
        
        if( temp == -1 ) // withdrawals for a week
        {
            if( girl->m_Withdrawals >= 20 )
                RemoveTrait( girl, "Fairy Dust Addict" );
            else
            {
                UpdateStat( girl, STAT_HAPPINESS, -30 );
                UpdateStat( girl, STAT_OBEDIENCE, -30 );
                UpdateStat( girl, STAT_HEALTH, -4 );
                
                if( !withdraw )
                {
                    girl->m_Withdrawals++;
                    withdraw = true;
                }
            }
        }
        else
        {
            UpdateStat( girl, STAT_HAPPINESS, 10 );
            UpdateTempStat( girl, STAT_LIBIDO, 5 );
            g_InvManager.Equip( girl, temp, false );
        }
    }
    
    if( HasTrait( girl, "Shroud Addict" ) )
    {
        int temp = HasItem( girl, "Shroud Mushroom" );
        
        if( temp == -1 ) // withdrawals for a week
        {
            if( girl->m_Withdrawals >= 20 )
                RemoveTrait( girl, "Shroud Addict" );
            else
            {
                UpdateStat( girl, STAT_HAPPINESS, -30 );
                UpdateStat( girl, STAT_OBEDIENCE, -30 );
                UpdateStat( girl, STAT_HEALTH, -4 );
                
                if( !withdraw )
                {
                    girl->m_Withdrawals++;
                    withdraw = true;
                }
            }
        }
        else
        {
            UpdateStat( girl, STAT_HAPPINESS, 10 );
            UpdateTempStat( girl, STAT_LIBIDO, 2 );
            g_InvManager.Equip( girl, temp, false );
        }
    }
    
    // sell crapy items
    for( int i = 0; i < 40; i++ ) // use a food item if it is in stock, and remove any bad things if disobedient
    {
        if( girl->m_Inventory[i] != nullptr )
        {
            int max = 0;
            
            switch( ( int )girl->m_Inventory[i]->m_Type )
            {
            case INVRING:
                if( max == 0 )
                    max = 8;
                    
            case INVDRESS:
                if( max == 0 )
                    max = 1;
                    
            case INVSHOES:
                if( max == 0 )
                    max = 1;
                    
            case INVNECKLACE:
                if( max == 0 )
                    max = 1;
                    
            case INVWEAPON:
                if( max == 0 )
                    max = 2;
                    
            case INVSMWEAPON:
                if( max == 0 )
                    max = 1;
                    
            case INVARMOR:
                if( max == 0 )
                    max = 1;
                    
            case INVARMBAND:
                if( max == 0 )
                    max = 2;
                    
                if( g_Girls.GetNumItemType( girl, girl->m_Inventory[i]->m_Type ) > max ) // MYR: Bug fix, was >=
                {
                    int nicerThan = g_Girls.GetWorseItem( girl, girl->m_Inventory[i]->m_Type, girl->m_Inventory[i]->m_Cost ); // find a worse item of the same type
                    
                    if( nicerThan != -1 )
                        g_Girls.SellInvItem( girl, nicerThan );
                }
                
                break;
            
            case INVFOOD: // Do not sell food stuff
            case INVMISC: // or misc stuff
                break;
            
            default:
                std::stringstream local_ss;
                local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
                g_LogFile.write( local_ss.str() );
                break;
            }
        }
    }
    
    int usedFood = ( g_Dice % 3 ) + 1;
    int usedFoodCount = 0;
    
    for( int i = 0; i < 40; i++ ) // use a food item if it is in stock, and remove any bad things if disobedient
    {
        sInventoryItem* curItem = girl->m_Inventory[i];
        
        if( curItem != nullptr )
        {
            if( ( curItem->m_Type == INVFOOD || curItem->m_Type == INVMAKEUP ) && usedFoodCount < usedFood )
            {
                if( !g_Dice.percent( curItem->m_GirlBuyChance ) ) // make sure she'd want it herself
                    continue;
                    
                // make sure there's some reason for her to use it
                bool useful = false;
                
                for( u_int j = 0; j < curItem->m_Effects.size(); j++ )
                {
                    sEffect* curEffect = &curItem->m_Effects[j];
                    
                    if( curEffect->m_Affects == sEffect::Nothing )
                    {
                        // really? it does nothing? sure, just for the hell of it
                        useful = true;
                        break;
                    }
                    else if( curEffect->m_Affects == sEffect::GirlStatus )
                    {
                        if( ( curEffect->m_Amount == 0 ) && girl->m_States & ( 1 << curEffect->m_EffectID ) )
                        {
                            // hmm... I don't think she would really want any of the available states, so...
                            useful = true;
                            break;
                        }
                    }
                    else if( curEffect->m_Affects == sEffect::Trait )
                    {
                        if( ( curEffect->m_Amount >= 1 ) != girl->has_trait( curEffect->m_Trait ) )
                        {
                            // girl has trait and item removes it, or doesn't have trait and item adds it
                            useful = true;
                            break;
                        }
                    }
                    else if( curEffect->m_Affects == sEffect::Stat )
                    {
                        unsigned int Stat = curEffect->m_EffectID;
                        
                        if( ( curEffect->m_Amount > 0 ) &&
                                (
                                    Stat == STAT_CHARISMA
                                    || Stat == STAT_HAPPINESS
                                    || Stat == STAT_FAME
                                    || Stat == STAT_LEVEL
                                    || Stat == STAT_ASKPRICE
                                    || Stat == STAT_EXP
                                    || Stat == STAT_BEAUTY
                                )
                          )
                        {
                            // even if this stat can't be increased further, she still wants it (call it vanity, greed, whatever)
                            useful = true;
                            break;
                        }
                        
                        if( ( curEffect->m_Amount > 0 ) && ( girl->m_Stats[Stat] < 100 ) &&
                                (
                                    Stat == STAT_LIBIDO
                                    || Stat == STAT_CONSTITUTION
                                    || Stat == STAT_INTELLIGENCE
                                    || Stat == STAT_CONFIDENCE
                                    || Stat == STAT_MANA
                                    || Stat == STAT_AGILITY
                                    || Stat == STAT_SPIRIT
                                    || Stat == STAT_HEALTH
                                )
                          )
                        {
                            // this stat increase would be good
                            useful = true;
                            break;
                        }
                        
                        if( ( curEffect->m_Amount < 0 ) && ( girl->m_Stats[Stat] > 0 ) &&
                                (
                                    Stat == STAT_AGE
                                    || Stat == STAT_TIREDNESS
                                )
                          )
                        {
                            // decreasing this stat would actually be good
                            useful = true;
                            break;
                        }
                    }
                    else if( curEffect->m_Affects == sEffect::Skill )
                    {
                        if( ( curEffect->m_Amount > 0 ) && ( girl->m_Stats[curEffect->m_EffectID] < 100 ) )
                        {
                            // skill would actually increase (wouldn't want to lose any skills)
                            useful = true;
                            break;
                        }
                    }
                }
                
                if( useful )
                {
                    // hey, this consumable item might actually be useful... gobble gobble gobble
                    g_InvManager.Equip( girl, i, false );
                    usedFoodCount++;
                }
            }
            
            // MYR: Girls shouldn't be able (IMHO) to take off things like control bracelets
            //else if(curItem->m_Badness > 20 && DisobeyCheck(girl, ACTION_GENERAL) && girl->m_EquipedItems[i] == 1)
            //{
            //  g_InvManager.Unequip(girl, i);
            //}
        }
    }
    
    // add the selling of items that are no longer needed here
}

bool cGirls::CanEquip( Girl* girl, int num, bool force )
{
    if( force )
        return true;
        
    switch( girl->m_Inventory[num]->m_Type )
    {
    case INVRING:   // worn on fingers (max 8)
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 8 )
            return false;
            
        break;
        
    case INVDRESS:  // Worn on body, (max 1)
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 1 )
            return false;
            
        break;
        
    case INVSHOES:  // worn on feet, (max 1)
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 1 )
            return false;
            
        break;
        
    case INVFOOD:   // Eaten, single use
        return true;
        break;
        
    case INVNECKLACE:   // worn on neck, (max 1)
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 1 )
            return false;
            
        break;
        
    case INVWEAPON: // equiped on body, (max 2)
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 2 )
            return false;
            
        break;
        
    case INVSMWEAPON: // hidden on body, (max 1)
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 1 )
            return false;
            
        break;
        
    case INVMAKEUP: // worn on face, single use
        return true;
        break;
        
    case INVARMOR:  // worn on body over dresses (max 1)
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 1 )
            return false;
            
        break;
        
    case INVMISC:       // these items don't usually do anythinig just random stuff girls might buy. The ones that do, cause a constant effect without having to be equiped
        return true;
        break;
        
    case INVARMBAND:    // (max 2), worn around arms
        if( GetNumItemEquiped( girl, girl->m_Inventory[num]->m_Type ) >= 2 )
            return false;
            
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    return true;
}

bool cGirls::IsItemEquipable( Girl* girl, int num )
{
    switch( girl->m_Inventory[num]->m_Type )
    {
    case sInventoryItem::Ring:
    case sInventoryItem::Dress:
    case sInventoryItem::Shoes:
    case sInventoryItem::Necklace:
    case sInventoryItem::Weapon:
    case sInventoryItem::Armor:
    case sInventoryItem::Armband:
        return true;
        
    case sInventoryItem::Food:
    case sInventoryItem::Makeup:
    case sInventoryItem::Misc:
    
        // adding this in explicitly - it was absent and returning false
        // this way gcc won't complain about the omission
    case sInventoryItem::SmWeapon:
        return false;
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        return false;
    }
    
    return false;
}

bool cGirls::EquipItem( Girl* girl, int num, bool force )
{
    if( CanEquip( girl, num, force ) )
    {
        g_InvManager.Equip( girl, num, force );
        return true;
    }
    
    return false;
}

bool cGirls::IsInvFull( Girl* girl )
{
    bool full = true;
    
    if( girl )
    {
        for( int i = 0; i < 40; i++ )
        {
            if( girl->m_Inventory[i] == nullptr )
            {
                full = false;
                break;
            }
        }
    }
    
    return full;
}

int cGirls::AddInv( Girl* girl, sInventoryItem* item )
{
    int i;
    
    for( i = 0; i < 40; i++ )
    {
        if( girl->m_Inventory[i] == nullptr )
        {
            girl->m_Inventory[i] = item;
            girl->m_NumInventory++;
            
            if( item->m_Type == INVMISC )
                EquipItem( girl, i, true );
                
            return i;  // MYR: return i for success, -1 for failure
        }
    }
    
    return -1;
}

bool cGirls::RemoveInvByNumber( Girl* girl, int Pos )
{
    // Girl inventories don't stack items
    if( girl->m_Inventory[Pos] != nullptr )
    {
        g_InvManager.Unequip( girl, Pos );
        girl->m_Inventory[Pos] = nullptr;
        girl->m_NumInventory--;
        return true;
    }
    
    return false;
}

void cGirls::SellInvItem( Girl* girl, int num )
{
    girl->m_Money += ( int )( ( float )girl->m_Inventory[num]->m_Cost * 0.5f );
    girl->m_NumInventory--;
    g_InvManager.Unequip( girl, num );
    girl->m_Inventory[num] = nullptr;
}

int cGirls::GetWorseItem( Girl* girl, int type, int cost )
{
    int ret = -1;
    
    if( girl->m_NumInventory == 0 )
        return -1;
        
    for( int i = 0; i < 40; i++ )
    {
        if( girl->m_Inventory[i] )
        {
            if( girl->m_Inventory[i]->m_Type == type && girl->m_Inventory[i]->m_Cost < cost )
            {
                ret = i;
                break;
            }
        }
    }
    
    return ret;
}

int cGirls::GetNumItemType( Girl* girl, int Type )
{
    if( girl->m_NumInventory == 0 )
        return 0;
        
    int num = 0;
    
    for( int i = 0; i < 40; i++ )
    {
        if( girl->m_Inventory[i] )
        {
            if( girl->m_Inventory[i]->m_Type == Type )
                num++;
        }
    }
    
    return num;
}

int cGirls::GetNumItemEquiped( Girl* girl, int Type )
{
    if( girl->m_NumInventory == 0 )
        return 0;
        
    int num = 0;
    
    for( int i = 0; i < 40; i++ )
    {
        if( girl->m_Inventory[i] )
        {
            if( girl->m_Inventory[i]->m_Type == Type && girl->m_EquipedItems[i] == 1 )
                num++;
        }
    }
    
    return num;
}

// ----- Trait

//int cGirls::UnapplyTraits(Girl* girl, sTrait* trait) //  WD: no idea as nothing uses the return value
void cGirls::UnapplyTraits( Girl* girl, sTrait* trait )
{
    /* WD:
    *   Added doOnce = false; to end of fn
    *   else the fn will allways abort
    */
    
    //  WD: don't know why it has to be static
    //static bool doOnce = false;
    
    //static int num=0;
    //if(doOnce)
    //{
    //  doOnce=false;
    //  return num;
    //}
    
    bool doOnce = false;
    
    if( trait )
        doOnce = true;
        
    for( int i = 0; i < girl->m_NumTraits || doOnce; i++ )
    {
        sTrait* tr = nullptr;
        
        if( doOnce )
            tr = trait;
        else
            tr = girl->m_Traits[i];
            
        if( tr == nullptr )
            continue;
            
            
        if( strcmp( tr->m_Name, "Big Boobs" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Abnormally Large Boobs", false, false, true ) )
                AddTrait( girl, "Small Boobs", false, false, true );
                
            UpdateStat( girl, STAT_BEAUTY, -10 );
            UpdateStat( girl, STAT_CONSTITUTION, -5 );
            UpdateStat( girl, STAT_AGILITY, 5 );
            UpdateStat( girl, STAT_CHARISMA, -2 );
        }
        
        else if( strcmp( tr->m_Name, "Abnormally Large Boobs" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Big Boobs", false, false, true ) )
                AddTrait( girl, "Small Boobs", false, false, true );
                
            UpdateStat( girl, STAT_BEAUTY, -10 );
            UpdateStat( girl, STAT_CONSTITUTION, -10 );
            UpdateStat( girl, STAT_AGILITY, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Small Boobs" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Big Boobs", false, false, true ) )
                AddTrait( girl, "Abnormally Large Boobs", false, false, true );
                
            UpdateStat( girl, STAT_AGILITY, -5 );
            UpdateStat( girl, STAT_CHARISMA, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Fast orgasms" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Fake orgasm expert", false, false, true ) )
                AddTrait( girl, "Slow orgasms", false, false, true );
                
            UpdateStat( girl, STAT_LIBIDO, -10 );
            UpdateSkill( girl, SKILL_ANAL, -10 );
            UpdateSkill( girl, SKILL_BDSM, -10 );
            UpdateSkill( girl, SKILL_NORMALSEX, -10 );
            UpdateSkill( girl, SKILL_BEASTIALITY, -10 );
            UpdateSkill( girl, SKILL_GROUP, -10 );
            UpdateSkill( girl, SKILL_LESBIAN, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Fake orgasm expert" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Fast orgasms", false, false, true ) )
                AddTrait( girl, "Slow orgasms", false, false, true );
                
            UpdateSkill( girl, SKILL_ANAL, -2 );
            UpdateSkill( girl, SKILL_BDSM, -2 );
            UpdateSkill( girl, SKILL_NORMALSEX, -2 );
            UpdateSkill( girl, SKILL_BEASTIALITY, -2 );
            UpdateSkill( girl, SKILL_GROUP, -2 );
            UpdateSkill( girl, SKILL_LESBIAN, -2 );
        }
        
        else if( strcmp( tr->m_Name, "Slow orgasms" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Fast orgasms", false, false, true ) )
                AddTrait( girl, "Fake orgasm expert", false, false, true );
                
            UpdateSkill( girl, SKILL_ANAL, 2 );
            UpdateSkill( girl, SKILL_BDSM, 2 );
            UpdateSkill( girl, SKILL_NORMALSEX, 2 );
            UpdateSkill( girl, SKILL_BEASTIALITY, 2 );
            UpdateSkill( girl, SKILL_GROUP, 2 );
            UpdateSkill( girl, SKILL_LESBIAN, 2 );
            UpdateStat( girl, STAT_CONFIDENCE, 2 );
        }
        
        else if( strcmp( tr->m_Name, "Different Colored Eyes" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Eye Patch", false, false, true ) )
                AddTrait( girl, "One Eye", false, false, true );
                
        }
        
        else if( strcmp( tr->m_Name, "Eye Patch" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Different Colored Eyes", false, false, true ) )
                AddTrait( girl, "One Eye", false, false, true );
                
            UpdateStat( girl, STAT_BEAUTY, 5 );
            UpdateStat( girl, STAT_CONSTITUTION, -5 );
        }
        
        else if( strcmp( tr->m_Name, "One Eye" ) == 0 )
        {
            // Can only have one trait added
            if( !AddTrait( girl, "Different Colored Eyes", false, false, true ) )
                AddTrait( girl, "Eye Patch", false, false, true );
                
            UpdateStat( girl, STAT_BEAUTY, 20 );
            UpdateStat( girl, STAT_CHARISMA, 5 );
            UpdateStat( girl, STAT_CONSTITUTION, -5 );
            UpdateStat( girl, STAT_SPIRIT, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Fearless" ) == 0 )
        {
            // OK Not incompatible traits
            AddTrait( girl, "Meek", false, false, true );
            AddTrait( girl, "Dependant", false, false, true );
            AddTrait( girl, "Nervous", false, false, true );
            
            UpdateStat( girl, STAT_PCFEAR, -200 );
            UpdateStat( girl, STAT_SPIRIT, 30 );
        }
        
        else if( strcmp( tr->m_Name, "Meek" ) == 0 )
        {
            // OK Not incompatible traits
            AddTrait( girl, "Aggressive", false, false, true );
            AddTrait( girl, "Fearless", false, false, true );
            
            UpdateStat( girl, STAT_CONFIDENCE, 20 );
            UpdateStat( girl, STAT_OBEDIENCE, -20 );
            UpdateStat( girl, STAT_SPIRIT, 20 );
        }
        
        /////////////////////////////////////////////////////////////
        else if( strcmp( tr->m_Name, "Good Kisser" ) == 0 )
        {
            UpdateStat( girl, STAT_LIBIDO, -2 );
            UpdateStat( girl, STAT_CONFIDENCE, -2 );
            UpdateStat( girl, STAT_CHARISMA, -2 );
            UpdateSkill( girl, SKILL_SERVICE, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Long Legs" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Perky Nipples" ) == 0 )
        {
            AddTrait( girl, "Puffy Nipples", false, false, true );
            UpdateStat( girl, STAT_BEAUTY, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Retarded" ) == 0 )
        {
// WD:      UpdateStat(girl,STAT_CONFIDENCE,20);
            UpdateStat( girl, STAT_SPIRIT, 20 );
            UpdateStat( girl, STAT_INTELLIGENCE, 50 );
// WD:      UpdateStat(girl,STAT_CONFIDENCE,40);
            UpdateStat( girl, STAT_CONFIDENCE, 60 );
        }
        
        else if( strcmp( tr->m_Name, "Malformed" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, 20 );
            UpdateStat( girl, STAT_SPIRIT, 20 );
            UpdateStat( girl, STAT_INTELLIGENCE, 10 );
            UpdateStat( girl, STAT_BEAUTY, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Puffy Nipples" ) == 0 )
        {
            AddTrait( girl, "Perky Nipples", false, false, true );
            UpdateStat( girl, STAT_BEAUTY, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Charming" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, -30 );
            UpdateStat( girl, STAT_BEAUTY, -15 );
        }
        
        else if( strcmp( tr->m_Name, "Charismatic" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, -50 );
            UpdateStat( girl, STAT_BEAUTY, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Construct" ) == 0 )
        {
            AddTrait( girl, "Half-Construct", false, false, true );
        }
        
        else if( strcmp( tr->m_Name, "Half-Construct" ) == 0 )
        {
            AddTrait( girl, "Construct", false, false, true );
            UpdateStat( girl, STAT_CONSTITUTION, -20 );
            UpdateSkill( girl, SKILL_COMBAT, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Fragile" ) == 0 )
        {
            AddTrait( girl, "Tough", false, false, true );
        }
        
        else if( strcmp( tr->m_Name, "Tough" ) == 0 )
        {
            AddTrait( girl, "Fragile", false, false, true );
        }
        
        else if( strcmp( tr->m_Name, "Mind Fucked" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, 50 );
            UpdateStat( girl, STAT_OBEDIENCE, -100 );
            UpdateStat( girl, STAT_SPIRIT, 50 );
        }
        
        else if( strcmp( tr->m_Name, "Fleet of Foot" ) == 0 )
        {
            UpdateStat( girl, STAT_AGILITY, -50 );
        }
        
        else if( strcmp( tr->m_Name, "Dependant" ) == 0 )
        {
            AddTrait( girl, "Fearless", false, false, true );
            UpdateStat( girl, STAT_SPIRIT, 30 );
            UpdateStat( girl, STAT_OBEDIENCE, -50 );
        }
        
        else if( strcmp( tr->m_Name, "Pessimist" ) == 0 )
        {
            AddTrait( girl, "Optimist", false, false, true );
            UpdateStat( girl, STAT_SPIRIT, 60 );
        }
        
        else if( strcmp( tr->m_Name, "Optimist" ) == 0 )
        {
            AddTrait( girl, "Pessimist", false, false, true );
        }
        
        else if( strcmp( tr->m_Name, "Iron Will" ) == 0 )
        {
            AddTrait( girl, "Broken Will", false, false, true );
            UpdateStat( girl, STAT_PCFEAR, 60 );
            UpdateStat( girl, STAT_SPIRIT, -60 );
            UpdateStat( girl, STAT_OBEDIENCE, 90 );
        }
        
        else if( strcmp( tr->m_Name, "Merciless" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Manly" ) == 0 )
        {
            AddTrait( girl, "Elegant", false, false, true );
            UpdateStat( girl, STAT_CONSTITUTION, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
            UpdateStat( girl, STAT_OBEDIENCE, 10 );
            UpdateStat( girl, STAT_SPIRIT, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Tsundere" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, -20 );
            UpdateStat( girl, STAT_OBEDIENCE, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Yandere" ) == 0 )
        {
            UpdateStat( girl, STAT_SPIRIT, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Lesbian" ) == 0 )
        {
            UpdateSkill( girl, SKILL_LESBIAN, -40 );
        }
        
        else if( strcmp( tr->m_Name, "Masochist" ) == 0 )
        {
            UpdateSkill( girl, SKILL_BDSM, -50 );
            UpdateStat( girl, STAT_CONSTITUTION, -50 );
            UpdateStat( girl, STAT_OBEDIENCE, -30 );
        }
        
        else if( strcmp( tr->m_Name, "Sadistic" ) == 0 )
        {
            UpdateSkill( girl, SKILL_BDSM, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Broken Will" ) == 0 )
        {
            AddTrait( girl, "Iron Will", false, false, true );
            UpdateStat( girl, STAT_SPIRIT, 20 );
            UpdateStat( girl, STAT_OBEDIENCE, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Nymphomaniac" ) == 0 )
        {
            UpdateStat( girl, STAT_LIBIDO, -20 );
            UpdateStat( girl, STAT_CONFIDENCE, -5 );
            UpdateStat( girl, STAT_CHARISMA, -5 );
            UpdateStat( girl, STAT_CONSTITUTION, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Elegant" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Sexy Air" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 5 );
            UpdateStat( girl, STAT_BEAUTY, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 2 );
        }
        
        else if( strcmp( tr->m_Name, "Great Arse" ) == 0 )
        {
            UpdateStat( girl, STAT_CONSTITUTION, -5 );
            UpdateStat( girl, STAT_BEAUTY, -5 );
            UpdateStat( girl, STAT_CHARISMA, -5 );
            UpdateSkill( girl, SKILL_ANAL, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Small Scars" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, 2 );
            UpdateStat( girl, STAT_CONSTITUTION, -2 );
            UpdateStat( girl, STAT_SPIRIT, 2 );
        }
        
        else if( strcmp( tr->m_Name, "Cool Scars" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, 2 );
            UpdateStat( girl, STAT_CONSTITUTION, 5 );
            UpdateStat( girl, STAT_SPIRIT, 5 );
            UpdateStat( girl, STAT_CHARISMA, 2 );
        }
        
        else if( strcmp( tr->m_Name, "Horrific Scars" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, 5 );
            UpdateStat( girl, STAT_CONSTITUTION, -10 );
            UpdateStat( girl, STAT_SPIRIT, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Cool Person" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, -5 );
            UpdateStat( girl, STAT_SPIRIT, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Nerd" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Clumsy" ) == 0 )
        {
            UpdateSkill( girl, SKILL_ANAL, 10 );
            UpdateSkill( girl, SKILL_BDSM, 10 );
            UpdateSkill( girl, SKILL_NORMALSEX, 10 );
            UpdateSkill( girl, SKILL_BEASTIALITY, 10 );
            UpdateSkill( girl, SKILL_GROUP, 10 );
            UpdateSkill( girl, SKILL_LESBIAN, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Cute" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, -10 );
            UpdateStat( girl, STAT_BEAUTY, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Strong" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Strong Magic" ) == 0 )
        {
            UpdateSkill( girl, SKILL_MAGIC, -20 );
            UpdateStat( girl, STAT_MANA, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Shroud Addict" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, 5 );
            UpdateStat( girl, STAT_CONFIDENCE, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Fairy Dust Addict" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
            UpdateStat( girl, STAT_OBEDIENCE, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Viras Blood Addict" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, 15 );
            UpdateStat( girl, STAT_OBEDIENCE, -20 );
            UpdateStat( girl, STAT_CHARISMA, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Aggressive" ) == 0 )
        {
            AddTrait( girl, "Meek", false, false, true );
            UpdateSkill( girl, SKILL_COMBAT, -10 );
            UpdateStat( girl, STAT_SPIRIT, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, -5 );
        }
        
        else if( strcmp( tr->m_Name, "Adventurer" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Assassin" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, -15 );
        }
        
        else if( strcmp( tr->m_Name, "Lolita" ) == 0 )
        {
            AddTrait( girl, "MILF", false, false, true );
            UpdateStat( girl, STAT_OBEDIENCE, -5 );
            UpdateStat( girl, STAT_CHARISMA, -10 );
            UpdateStat( girl, STAT_BEAUTY, -20 );
        }
        
        else if( strcmp( tr->m_Name, "MILF" ) == 0 )
        {
            AddTrait( girl, "Lolita", false, false, true );
        }
        
        else if( strcmp( tr->m_Name, "Nervous" ) == 0 )
        {
            AddTrait( girl, "Fearless", false, false, true );
            UpdateStat( girl, STAT_OBEDIENCE, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Great Figure" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, -10 );
            UpdateStat( girl, STAT_BEAUTY, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Incorporial" ) == 0 )
        {
            RemoveTrait( girl, "Sterile" );
        }
        
        else if( strcmp( tr->m_Name, "Quick Learner" ) == 0 )
        {
            AddTrait( girl, "Slow Learner", false, false, true );
        }
        else if( strcmp( tr->m_Name, "Slow Learner" ) == 0 )
        {
            AddTrait( girl, "Quick Learner", false, false, true );
        }
        
        
        if( doOnce )
        {
            // WD:  Added to stop fn from aborting
            //doOnce = false;
            //return i;
            break;
        }
    }
    
//  return -1;
}

// If a girl enjoys a job enough, she has a chance of gaining traits associated with it
// (Made a FN out of code appearing in WorkExploreCatacombs etc...)

bool cGirls::PossiblyGainNewTrait( Girl* girl, std::string Trait, int Threshold, int ActionType, std::string Message, bool /*DayNight*/ )
{
    if( girl->m_Enjoyment[ActionType] > Threshold && !girl->has_trait( Trait ) )
    {
        int chance = ( girl->m_Enjoyment[ActionType] - Threshold );
        
        if( g_Dice.percent( chance ) )
        {
            girl->add_trait( Trait, false );
            girl->m_Events.AddMessage( Message, IMGTYPE_PROFILE, EVENT_WARNING );
            return true;
        }
    }
    
    return false;
}

void cGirls::ApplyTraits( Girl* girl, sTrait* trait, bool rememberflag )
{
    /* WD:
    *   Added doOnce = false; to end of fn
    *   else the fn will allways abort
    */
    
    //  WD: don't know why it has to be static
    //static bool doOnce = false;
    
    //if(doOnce)
    //  return;
    
    bool doOnce = false;
    
    if( trait )
        doOnce = true;
        
    for( int i = 0; i < girl->m_NumTraits || doOnce; i++ )
    {
        sTrait* tr = nullptr;
        
        if( doOnce )
            tr = trait;
        else
            tr = girl->m_Traits[i];
            
        if( tr == nullptr )
            continue;
            
        if( strcmp( tr->m_Name, "Big Boobs" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Abnormally Large Boobs", true, true))
            //  RemoveTrait(girl, "Small Boobs", false, true);
            //else
            //  RemoveTrait(girl, "Small Boobs", true, true);
            
            RemoveTrait( girl, "Abnormally Large Boobs", rememberflag, true );
            RemoveTrait( girl, "Small Boobs", rememberflag, true );
            
            UpdateStat( girl, STAT_BEAUTY, 10 );
            UpdateStat( girl, STAT_CONSTITUTION, 5 );
            UpdateStat( girl, STAT_AGILITY, -5 );
            UpdateStat( girl, STAT_CHARISMA, 2 );
        }
        
        else if( strcmp( tr->m_Name, "Abnormally Large Boobs" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Big Boobs", true, true))
            //  RemoveTrait(girl, "Small Boobs", false, true);
            //else
            //  RemoveTrait(girl, "Small Boobs", true, true);
            
            RemoveTrait( girl, "Big Boobs", rememberflag, true );
            RemoveTrait( girl, "Small Boobs", rememberflag, true );
            
            UpdateStat( girl, STAT_BEAUTY, 10 );
            UpdateStat( girl, STAT_CONSTITUTION, 10 );
            UpdateStat( girl, STAT_AGILITY, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Small Boobs" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Big Boobs", true, true))
            //  RemoveTrait(girl, "Abnormally Large Boobs", false, true);
            //else
            //  RemoveTrait(girl, "Abnormally Large Boobs", true, true);
            
            RemoveTrait( girl, "Big Boobs", rememberflag, true );
            RemoveTrait( girl, "Abnormally Large Boobs", rememberflag, true );
            
            UpdateStat( girl, STAT_AGILITY, 5 );
            UpdateStat( girl, STAT_CHARISMA, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Fast orgasms" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Fake orgasm expert", true, true))
            //  RemoveTrait(girl, "Slow orgasms", false, true);
            //else
            //  RemoveTrait(girl, "Slow orgasms", true, true);
            
            RemoveTrait( girl, "Fake orgasm expert", rememberflag, true );
            RemoveTrait( girl, "Slow orgasms", rememberflag, true );
            
            UpdateStat( girl, STAT_LIBIDO, 10 );
            UpdateSkill( girl, SKILL_ANAL, 10 );
            UpdateSkill( girl, SKILL_BDSM, 10 );
            UpdateSkill( girl, SKILL_NORMALSEX, 10 );
            UpdateSkill( girl, SKILL_BEASTIALITY, 10 );
            UpdateSkill( girl, SKILL_GROUP, 10 );
            UpdateSkill( girl, SKILL_LESBIAN, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Fake orgasm expert" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Fast orgasms", true, true))
            //  RemoveTrait(girl, "Slow orgasms", false, true);
            //else
            //  RemoveTrait(girl, "Slow orgasms", true, true);
            
            RemoveTrait( girl, "Slow orgasms", rememberflag, true );
            RemoveTrait( girl, "Fast orgasms", rememberflag, true );
            
            UpdateSkill( girl, SKILL_ANAL, 2 );
            UpdateSkill( girl, SKILL_BDSM, 2 );
            UpdateSkill( girl, SKILL_NORMALSEX, 2 );
            UpdateSkill( girl, SKILL_BEASTIALITY, 2 );
            UpdateSkill( girl, SKILL_GROUP, 2 );
            UpdateSkill( girl, SKILL_LESBIAN, 2 );
        }
        
        
        else if( strcmp( tr->m_Name, "Slow orgasms" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Fast orgasms", true, true))
            //  RemoveTrait(girl, "Fake orgasm expert", false, true);
            //else
            //  RemoveTrait(girl, "Fake orgasm expert", true, true);
            
            RemoveTrait( girl, "Fake orgasm expert", rememberflag, true );
            RemoveTrait( girl, "Fast orgasms", rememberflag, true );
            
            UpdateSkill( girl, SKILL_ANAL, -2 );
            UpdateSkill( girl, SKILL_BDSM, -2 );
            UpdateSkill( girl, SKILL_NORMALSEX, -2 );
            UpdateSkill( girl, SKILL_BEASTIALITY, -2 );
            UpdateSkill( girl, SKILL_GROUP, -2 );
            UpdateSkill( girl, SKILL_LESBIAN, -2 );
            UpdateStat( girl, STAT_CONFIDENCE, -2 );
        }
        
        else if( strcmp( tr->m_Name, "Different Colored Eyes" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Eye Patch", true, true))
            //  RemoveTrait(girl, "One Eye", false, true);
            //else
            //  RemoveTrait(girl, "One Eye", true, true);
            
            RemoveTrait( girl, "Eye Patch", rememberflag, true );
            RemoveTrait( girl, "One Eye", rememberflag, true );
        }
        
        else if( strcmp( tr->m_Name, "Eye Patch" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "One Eye", true, true))
            //  RemoveTrait(girl, "Different Colored Eyes", false, true);
            //else
            //  RemoveTrait(girl, "Different Colored Eyes", true, true);
            
            RemoveTrait( girl, "Different Colored Eyes", rememberflag, true );
            RemoveTrait( girl, "One Eye", rememberflag, true );
            
            UpdateStat( girl, STAT_BEAUTY, -5 );
            UpdateStat( girl, STAT_CONSTITUTION, 5 );
        }
        
        else if( strcmp( tr->m_Name, "One Eye" ) == 0 )
        {
            // should only have one trait but lets make sure
            //if (RemoveTrait(girl, "Different Colored Eyes", true, true))
            //  RemoveTrait(girl, "Eye Patch", false, true);
            //else
            //  RemoveTrait(girl, "Eye Patch", true, true);
            
            RemoveTrait( girl, "Eye Patch", rememberflag, true );
            RemoveTrait( girl, "Different Colored Eyes", rememberflag, true );
            
            UpdateStat( girl, STAT_BEAUTY, -20 );
            UpdateStat( girl, STAT_CHARISMA, -5 );
            UpdateStat( girl, STAT_CONSTITUTION, 5 );
            UpdateStat( girl, STAT_SPIRIT, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Fearless" ) == 0 )
        {
            // OK Not incompatible traits
            RemoveTrait( girl, "Meek", rememberflag, true );
            RemoveTrait( girl, "Dependant", rememberflag, true );
            RemoveTrait( girl, "Nervous", rememberflag, true );
            
            UpdateStat( girl, STAT_PCFEAR, -200 );
            UpdateStat( girl, STAT_SPIRIT, 30 );
        }
        
        else if( strcmp( tr->m_Name, "Meek" ) == 0 )
        {
            // OK Not incompatible traits
            RemoveTrait( girl, "Aggressive", rememberflag, true );
            RemoveTrait( girl, "Fearless", rememberflag, true );
            
            UpdateStat( girl, STAT_CONFIDENCE, -20 );
            UpdateStat( girl, STAT_OBEDIENCE, 20 );
            UpdateStat( girl, STAT_SPIRIT, -20 );
        }
        
        
        /////////////////////////////////////////////////////////////
        else if( strcmp( tr->m_Name, "Good Kisser" ) == 0 )
        {
            UpdateStat( girl, STAT_LIBIDO, 2 );
            UpdateStat( girl, STAT_CONFIDENCE, 2 );
            UpdateStat( girl, STAT_CHARISMA, 2 );
            UpdateSkill( girl, SKILL_SERVICE, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Retarded" ) == 0 )
        {
//  WD:     UpdateStat(girl,STAT_CONFIDENCE,-20);
            UpdateStat( girl, STAT_SPIRIT, -20 );
            UpdateStat( girl, STAT_INTELLIGENCE, -50 );
//  WD:     UpdateStat(girl,STAT_CONFIDENCE,-40);
            UpdateStat( girl, STAT_CONFIDENCE, -60 );
        }
        
        else if( strcmp( tr->m_Name, "Malformed" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, -20 );
            UpdateStat( girl, STAT_SPIRIT, -20 );
            UpdateStat( girl, STAT_INTELLIGENCE, -10 );
            UpdateStat( girl, STAT_BEAUTY, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Long Legs" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Perky Nipples" ) == 0 )
        {
            RemoveTrait( girl, "Puffy Nipples", rememberflag, true );
            UpdateStat( girl, STAT_BEAUTY, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Puffy Nipples" ) == 0 )
        {
            RemoveTrait( girl, "Perky Nipples", rememberflag, true );
            UpdateStat( girl, STAT_BEAUTY, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Charismatic" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 50 );
            UpdateStat( girl, STAT_BEAUTY, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Charming" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 30 );
            UpdateStat( girl, STAT_BEAUTY, 15 );
        }
        
        else if( strcmp( tr->m_Name, "Construct" ) == 0 )
        {
            RemoveTrait( girl, "Half-Construct", rememberflag, true );
        }
        
        else if( strcmp( tr->m_Name, "Half-Construct" ) == 0 )
        {
            RemoveTrait( girl, "Construct", rememberflag, true );
            UpdateStat( girl, STAT_CONSTITUTION, 20 );
            UpdateSkill( girl, SKILL_COMBAT, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Fragile" ) == 0 )
        {
            RemoveTrait( girl, "Tough", rememberflag, true );
        }
        
        else if( strcmp( tr->m_Name, "Tough" ) == 0 )
        {
            RemoveTrait( girl, "Fragile", rememberflag, true );
        }
        
        else if( strcmp( tr->m_Name, "Fleet of Foot" ) == 0 )
        {
            UpdateStat( girl, STAT_AGILITY, 50 );
        }
        
        else if( strcmp( tr->m_Name, "Dependant" ) == 0 )
        {
            RemoveTrait( girl, "Fearless", rememberflag, true );
            UpdateStat( girl, STAT_SPIRIT, -30 );
            UpdateStat( girl, STAT_OBEDIENCE, 50 );
        }
        
        else if( strcmp( tr->m_Name, "Pessimist" ) == 0 )
        {
            RemoveTrait( girl, "Optimist", rememberflag, true );
            UpdateStat( girl, STAT_SPIRIT, -60 );
        }
        
        else if( strcmp( tr->m_Name, "Optimist" ) == 0 )
        {
            RemoveTrait( girl, "Pessimist", rememberflag, true );
        }
        
        else if( strcmp( tr->m_Name, "Iron Will" ) == 0 )
        {
            RemoveTrait( girl, "Broken Will", rememberflag, true );
            UpdateStat( girl, STAT_PCFEAR, -60 );
            UpdateStat( girl, STAT_SPIRIT, 60 );
            UpdateStat( girl, STAT_OBEDIENCE, -90 );
        }
        
        else if( strcmp( tr->m_Name, "Merciless" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Manly" ) == 0 )
        {
            RemoveTrait( girl, "Elegant", rememberflag, true );
            UpdateStat( girl, STAT_CONSTITUTION, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
            UpdateStat( girl, STAT_OBEDIENCE, -10 );
            UpdateStat( girl, STAT_SPIRIT, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Mind Fucked" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, -50 );
            UpdateStat( girl, STAT_OBEDIENCE, 100 );
            UpdateStat( girl, STAT_SPIRIT, -50 );
        }
        
        else if( strcmp( tr->m_Name, "Tsundere" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, 20 );
            UpdateStat( girl, STAT_OBEDIENCE, -20 );
        }
        
        else if( strcmp( tr->m_Name, "Yandere" ) == 0 )
        {
            UpdateStat( girl, STAT_SPIRIT, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Lesbian" ) == 0 )
        {
            UpdateSkill( girl, SKILL_LESBIAN, 40 );
        }
        
        else if( strcmp( tr->m_Name, "Masochist" ) == 0 )
        {
            UpdateSkill( girl, SKILL_BDSM, 50 );
            UpdateStat( girl, STAT_CONSTITUTION, 50 );
            UpdateStat( girl, STAT_OBEDIENCE, 30 );
        }
        
        else if( strcmp( tr->m_Name, "Sadistic" ) == 0 )
        {
            UpdateSkill( girl, SKILL_BDSM, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Broken Will" ) == 0 )
        {
            RemoveTrait( girl, "Iron Will", rememberflag, true );
            UpdateStat( girl, STAT_SPIRIT, -100 );
            UpdateStat( girl, STAT_OBEDIENCE, 100 );
        }
        
        else if( strcmp( tr->m_Name, "Nymphomaniac" ) == 0 )
        {
            UpdateStat( girl, STAT_LIBIDO, 20 );
            UpdateStat( girl, STAT_CONFIDENCE, 5 );
            UpdateStat( girl, STAT_CHARISMA, 5 );
            UpdateStat( girl, STAT_CONSTITUTION, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Elegant" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Sexy Air" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 5 );
            UpdateStat( girl, STAT_BEAUTY, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 2 );
        }
        
        else if( strcmp( tr->m_Name, "Great Arse" ) == 0 )
        {
            UpdateStat( girl, STAT_CONSTITUTION, 5 );
            UpdateStat( girl, STAT_BEAUTY, 5 );
            UpdateStat( girl, STAT_CHARISMA, 5 );
            UpdateSkill( girl, SKILL_ANAL, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Small Scars" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, -2 );
            UpdateStat( girl, STAT_CONSTITUTION, 2 );
            UpdateStat( girl, STAT_SPIRIT, -2 );
        }
        
        else if( strcmp( tr->m_Name, "Cool Scars" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, 2 );
            UpdateStat( girl, STAT_CONSTITUTION, 5 );
            UpdateStat( girl, STAT_SPIRIT, 5 );
            UpdateStat( girl, STAT_CHARISMA, 2 );
        }
        
        else if( strcmp( tr->m_Name, "Horrific Scars" ) == 0 )
        {
            UpdateStat( girl, STAT_BEAUTY, -5 );
            UpdateStat( girl, STAT_CONSTITUTION, 10 );
            UpdateStat( girl, STAT_SPIRIT, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Cool Person" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 5 );
            UpdateStat( girl, STAT_SPIRIT, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Nerd" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Clumsy" ) == 0 )
        {
            UpdateSkill( girl, SKILL_ANAL, -10 );
            UpdateSkill( girl, SKILL_BDSM, -10 );
            UpdateSkill( girl, SKILL_NORMALSEX, -10 );
            UpdateSkill( girl, SKILL_BEASTIALITY, -10 );
            UpdateSkill( girl, SKILL_GROUP, -10 );
            UpdateSkill( girl, SKILL_LESBIAN, -10 );
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Cute" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 10 );
            UpdateStat( girl, STAT_BEAUTY, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Strong" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Strong Magic" ) == 0 )
        {
            UpdateSkill( girl, SKILL_MAGIC, 20 );
            UpdateStat( girl, STAT_MANA, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Shroud Addict" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, -5 );
            UpdateStat( girl, STAT_CONFIDENCE, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Fairy Dust Addict" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
            UpdateStat( girl, STAT_OBEDIENCE, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Viras Blood Addict" ) == 0 )
        {
            UpdateStat( girl, STAT_CONFIDENCE, -15 );
            UpdateStat( girl, STAT_OBEDIENCE, 20 );
            UpdateStat( girl, STAT_CHARISMA, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Aggressive" ) == 0 )
        {
            RemoveTrait( girl, "Meek", rememberflag, true );
            UpdateSkill( girl, SKILL_COMBAT, 10 );
            UpdateStat( girl, STAT_SPIRIT, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, 5 );
        }
        
        else if( strcmp( tr->m_Name, "Adventurer" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, 10 );
        }
        
        else if( strcmp( tr->m_Name, "Assassin" ) == 0 )
        {
            UpdateSkill( girl, SKILL_COMBAT, 15 );
        }
        
        else if( strcmp( tr->m_Name, "Lolita" ) == 0 )
        {
            RemoveTrait( girl, "MILF", rememberflag, true );
            UpdateStat( girl, STAT_OBEDIENCE, 5 );
            UpdateStat( girl, STAT_CHARISMA, 10 );
            UpdateStat( girl, STAT_BEAUTY, 20 );
        }
        
        else if( strcmp( tr->m_Name, "MILF" ) == 0 )
        {
            RemoveTrait( girl, "Lolita", rememberflag, true );
        }
        
        else if( strcmp( tr->m_Name, "Nervous" ) == 0 )
        {
            RemoveTrait( girl, "Fearless", rememberflag, true );
            UpdateStat( girl, STAT_OBEDIENCE, 10 );
            UpdateStat( girl, STAT_CONFIDENCE, -10 );
        }
        
        else if( strcmp( tr->m_Name, "Great Figure" ) == 0 )
        {
            UpdateStat( girl, STAT_CHARISMA, 10 );
            UpdateStat( girl, STAT_BEAUTY, 20 );
        }
        
        else if( strcmp( tr->m_Name, "Incorporial" ) == 0 )
        {
            AddTrait( girl, "Sterile" );
        }
        
        else if( strcmp( tr->m_Name, "Quick Learner" ) == 0 )
        {
            RemoveTrait( girl, "Slow Learner", rememberflag, true );
        }
        
        else if( strcmp( tr->m_Name, "Slow Learner" ) == 0 )
        {
            RemoveTrait( girl, "Quick Learner", rememberflag, true );
        }
        
        if( doOnce )
        {
            // WD:  Added to stop fn from aborting
            // doOnce = false;
            break;
        }
    }
}

bool cGirls::HasRememberedTrait( Girl* girl, std::string trait )
{
    for( int i = 0; i < MAXNUM_TRAITS * 2; i++ )
    {
        if( girl->m_RememTraits[i] )
        {
            if( trait.compare( girl->m_RememTraits[i]->m_Name ) == 0 )
                return true;
        }
    }
    
    return false;
}

bool cGirls::HasTrait( Girl* girl, std::string trait )
{
    for( int i = 0; i < MAXNUM_TRAITS; i++ )
    {
        if( girl->m_Traits[i] )
        {
            if( trait.compare( girl->m_Traits[i]->m_Name ) == 0 )
                return true;
        }
    }
    
    return false;
}

void cGirls::RemoveRememberedTrait( Girl* girl, std::string name )
{
    sTrait* trait = g_Traits.GetTrait( name );
    
    for( int i = 0; i < MAXNUM_TRAITS * 2; i++ ) // remove the traits
    {
        if( girl->m_RememTraits[i] )
        {
            if( girl->m_RememTraits[i] == trait )
            {
                girl->m_NumRememTraits--;
                girl->m_RememTraits[i] = nullptr;
                return;
            }
        }
    }
}

void cGirls::RemoveAllRememberedTraits( Girl* girl )
{
    /*
     *  WD: Remove all traits to for new girl creation
     *      is required after using ApplyTraits() & UnApplyTraits()
     *      can create remembered traits especially random girls
     */
    
    for( int i = 0; i < MAXNUM_TRAITS * 2; i++ )
    {
        // WD:  This should be faster 121 writes instead of 120 reads and (120 + numTraits) writes
        girl->m_RememTraits[i] = nullptr;
        
        //if(girl->m_RememTraits[i])
        //{
        //  girl->m_NumRememTraits--;
        //  girl->m_RememTraits[i] = 0;
        //}
    }
    
    girl->m_NumRememTraits = 0;
}

bool cGirls::RemoveTrait( Girl* girl, std::string name, bool addrememberlist, bool force )
{
    /*
     *  WD: Added logic for remembered trait
     *
     *      addrememberlist = true when trait is
     *      to be added to the remember list
     *
     *      force = true when you must force
     *      removal of the trait if active even if
     *      it is on the remember list.
     *
     *      Returns true if there was a active trait
     *      or remember list entry removed
     *
     *      This should fix items with duplicate
     *      traits and loss of original trait if
     *      overwritten by a trait from an item
     *      that is later removed
     */
    
    
    
    bool hasRemTrait = HasRememberedTrait( girl, name );
    
    if( !HasTrait( girl, name ) )                       // WD:  no active trait to remove
#if 0
        if( !force && hasRemTrait )                     // WD:  try remembered trait
        {
            RemoveRememberedTrait( girl, name );
            return true;
        }
        else
            return false;
            
#else
    {
        if( hasRemTrait )                               // WD:  try remembered trait
        {
            RemoveRememberedTrait( girl, name );
            return true;
        }
        else
            return false;
    }
#endif
            
    //  WD: has remembered trait so don't touch active trait
    //      unless we are forcing removal of active trait
    if( !force && hasRemTrait )
    {
        RemoveRememberedTrait( girl, name );
        return true;
    }
    
    //  WD: save trait to remember list before removing
    if( addrememberlist )
        AddRememberedTrait( girl, name );
        
    //  WD: Remove trait
    sTrait* trait = g_Traits.GetTrait( name );
    
    for( int i = 0; i < MAXNUM_TRAITS; i++ )    // remove the traits
    {
        if( girl->m_Traits[i] )
        {
            if( girl->m_Traits[i] == trait )
            {
                girl->m_NumTraits--;
                UnapplyTraits( girl, girl->m_Traits[i] );
                
                if( girl->m_TempTrait[i] > 0 )
                    girl->m_TempTrait[i] = 0;
                    
                girl->m_Traits[i] = nullptr;
                return true;
            }
        }
    }
    
    return false;
}

void cGirls::AddRememberedTrait( Girl* girl, std::string name )
{
    for( int i = 0; i < MAXNUM_TRAITS * 2; i++ ) // add the traits
    {
        if( girl->m_RememTraits[i] == nullptr )
        {
            girl->m_NumRememTraits++;
            girl->m_RememTraits[i] = g_Traits.GetTrait( name );
            return;
        }
    }
}

bool cGirls::AddTrait( Girl* girl, std::string name, bool temp, bool removeitem, bool inrememberlist )
{
    /*
     *  WD: Added logic for remembered trait
     *
     *      removeitem = true Will add to Remember
     *      trait list if the trait is allready active
     *      Used with items / efects may be removed
     *      later eg items - rings
     *
     *      inrememberlist = true only add trait if
     *      exists in the remember list and remove
     *      from the list. Use mainly with unequiping
     *      items and Trait overiding in ApplyTraits()
     *
     *      Returns true if trait mad active or added
     *      trait remember list.
     *
     *      This should fix items with duplicate
     *      traits and loss of original trait if
     *      overwritten by a trait from an item
     *      that is later removed
     *
     */
    
    
    if( HasTrait( girl, name ) )
    {
        if( removeitem )                            //  WD: Overwriting existing trait with removable item / effect
            AddRememberedTrait( girl, name );       //  WD: Save trait for when item is removed
            
        return true;
    }
    
    if( inrememberlist )                            // WD: Add trait only if it is in the Remember List
    {
        if( HasRememberedTrait( girl, name ) )
        {
            RemoveRememberedTrait( girl, name );
        }
        
        else
            return false;                           //  WD: No trait to add
    }
    
    for( int i = 0; i < MAXNUM_TRAITS; i++ )        // add the trait
    {
        if( girl->m_Traits[i] == nullptr )
        {
            if( temp )
                girl->m_TempTrait[i] = 20;
                
            girl->m_NumTraits++;
            girl->m_Traits[i] = g_Traits.GetTrait( name );
            ApplyTraits( girl, girl->m_Traits[i], removeitem );
            return true;
        }
    }
    
    return false;
}

#if 0

/*
 * WD: Sanity checks not allways working as apply traits is called
 *  direclty by the load coad bypassing these checks
 *  also some of the checks in applytraits() and unapplytraits() are not here
 */
// Sanity checks
if( HasTrait( girl, "Manly" ) && HasTrait( girl, "Elegant" ) )
    RemoveTrait( girl, "Elegant" );

if( HasTrait( girl, "MILF" ) && HasTrait( girl, "Lolita" ) )
    RemoveTrait( girl, "Lolita" );

if( HasTrait( girl, "Quick Learner" ) && HasTrait( girl, "Slow Learner" ) )
    RemoveTrait( girl, "Slow Learner" );

if( HasTrait( girl, "Small Boobs" ) && ( HasTrait( girl, "Big Boobs" ) || HasTrait( girl, "Abnormally Large Boobs" ) ) )
    RemoveTrait( girl, "Small Boobs" );

if( HasTrait( girl, "Big Boobs" ) && HasTrait( girl, "Abnormally Large Boobs" ) )
    RemoveTrait( girl, "Big Boobs" );

if( HasTrait( girl, "Iron Will" ) && HasTrait( girl, "Broken Will" ) )
    RemoveTrait( girl, "Iron Will" );

if( HasTrait( girl, "Fast Orgasms" ) && HasTrait( girl, "Slow Orgasms" ) )
    RemoveTrait( girl, "Slow Orgasms" );

if( HasTrait( girl, "Aggressive" ) && HasTrait( girl, "Meek" ) )
    RemoveTrait( girl, "Meek" );

if( HasTrait( girl, "Fearless" ) && ( HasTrait( girl, "Dependant" ) || HasTrait( girl, "Nervous" ) || HasTrait( girl, "Meek" ) ) )
{
    if( HasTrait( girl, "Dependant" ) )
        RemoveTrait( girl, "Dependant" );
        
    if( HasTrait( girl, "Nervous" ) )
        RemoveTrait( girl, "Nervous" );
        
    if( HasTrait( girl, "Meek" ) )
        RemoveTrait( girl, "Meek" );
}

if( HasTrait( girl, "Optimist" ) && HasTrait( girl, "Pessimist" ) )
    RemoveTrait( girl, "Pessimist" );

if( HasTrait( girl, "Tough" ) && HasTrait( girl, "Fragile" ) )
    RemoveTrait( girl, "Fragile" );

if( HasTrait( girl, "Different Colored Eyes" ) && ( HasTrait( girl, "One Eye" ) || HasTrait( girl, "Eye Patch" ) ) )
{
    if( HasTrait( girl, "One Eye" ) )
        RemoveTrait( girl, "One Eye" );
        
    if( HasTrait( girl, "Eye Patch" ) )
        RemoveTrait( girl, "Eye Patch" );
}

#endif

void cGirls::updateTempTraits( Girl* girl )
{
    /*
     *   Update temp traits and remove expired traits
     */
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    for( int i = 0; i < MAXNUM_TRAITS; i++ )
    {
        if( girl->m_Traits[i] && girl->m_TempTrait[i] > 0 )
        {
            girl->m_TempTrait[i]--;
            
            if( girl->m_TempTrait[i] == 0 )
                g_Girls.RemoveTrait( girl, girl->m_Traits[i]->m_Name );
        }
    }
}

void cGirls::updateHappyTraits( Girl* girl )
{
    /*
     *   Update happiness for trait affects
     */
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    if( girl->has_trait( "Optimist" ) )
        girl->happiness( 5 );
        
    if( girl->has_trait( "Pessimist" ) )
    {
        girl->happiness( -5 );
        
        if( girl->happiness() <= 0 )
        {
            std::string msg = girl->m_Realname + " has killed herself since she was unhappy and depressed.";
            girl->m_Events.AddMessage( msg, IMGTYPE_DEATH, EVENT_DANGER );
            g_MessageQue.AddToQue( msg, 1 );
            //g_Girls.SetStat(girl, STAT_HEALTH, 0);
        }
    }
}

// ----- Sex

void cGirls::GirlFucks( Girl* girl, int /*DayNight*/, sCustomer* customer, bool group, std::string& message, u_int& SexType )
{
    bool good = false;
    bool contraception = false;
    
    // Start the customers unhappiness/happiness bad sex decreases, good sex inceases
    if( HasTrait( girl, "Fast orgasms" ) ) // has priority
        customer->m_Stats[STAT_HAPPINESS] += 15;
    else if( HasTrait( girl, "Slow orgasms" ) )
        customer->m_Stats[STAT_HAPPINESS] -= 10;
        
    if( HasTrait( girl, "Psychic" ) )
        customer->m_Stats[STAT_HAPPINESS] += 10;
        
    if( HasTrait( girl, "Fake Orgasms" ) )
        customer->m_Stats[STAT_HAPPINESS] += 15;
        
    if( HasTrait( girl, "Abnormally Large Boobs" ) ) // WD: added
        customer->m_Stats[STAT_HAPPINESS] += 15;
    else if( HasTrait( girl, "Big Boobs" ) )        // WD: Fixed Spelling
        customer->m_Stats[STAT_HAPPINESS] += 10;
        
        
    girl->m_NumCusts += ( int )customer->m_Amount;
    
    if( group )
    {
        // the customer will be an average in all skills for the customers involved in the sex act
        SexType = SKILL_GROUP;
    }
    else    // Any other sex acts
        SexType = customer->m_SexPref;
        
    // If the girls skill < 50 then it will be unsatisfying otherwise it will be satisfying
    if( GetSkill( girl, SexType ) < 50 )
        customer->m_Stats[STAT_HAPPINESS] -= ( 100 - GetSkill( girl, SexType ) ) / 5;
    else
        customer->m_Stats[STAT_HAPPINESS] += GetSkill( girl, SexType ) / 5;
        
    // If the girl is famous then he will be slightly happier
    customer->m_Stats[STAT_HAPPINESS] += GetStat( girl, STAT_FAME ) / 5;
    
    // her service ability will also make him happier (I.e. does she help clean him well)
    customer->m_Stats[STAT_HAPPINESS] += GetSkill( girl, SKILL_SERVICE ) / 10;
    
    // her magic ability can make him think he enjoyed it more if she has mana
    // WD: only if Customer is not Max Happy else Whores never have any mana
    if( customer->m_Stats[STAT_HAPPINESS] < 100 && GetStat( girl, STAT_MANA ) > 20 )
    {
        customer->m_Stats[STAT_HAPPINESS] += GetSkill( girl, SKILL_MAGIC ) / 10;
        UpdateStat( girl, STAT_MANA, -20 );
    }
    
    message = girl->m_Realname;
    
    switch( SexType )
    {
    case SKILL_ANAL:
        if( GetSkill( girl, SexType ) < 20 )
            message += " found it difficult to get it in but painfully allowed the customer to fuck her in her tight ass.";
        else if( GetSkill( girl, SexType ) < 40 )
            message += " had to relax somewhat but had the customer fucking her in her ass.";
        else if( GetSkill( girl, SexType ) < 60 )
            message += " found it easier going with the customer fucking her in her ass.";
        else if( GetSkill( girl, SexType ) < 80 )
            message += " had the customer's cock go in easy. She found his cock in her ass a very pleasurable experience.";
        else
            message += " came hard as the customer fucked her ass.";
            
        break;
        
    case SKILL_BDSM:
        if( GetSkill( girl, SexType ) < 40 )
            message += " was frightened by being tied up and having pain inflicted on her.";
        else if( GetSkill( girl, SexType ) < 60 )
            message += " was a little turned on by being tied up and having the customer hurting her.";
        else if( GetSkill( girl, SexType ) < 80 )
            message += " was highly aroused by the pain and bondage, even more so when fucking at the same time.";
        else
            message += GetRandomBDSMString();
            
        //          " her screams of pain mixed with pleasure could be heard throughout the building as she came over and over during the bondage session.";
        break;
        
    case SKILL_NORMALSEX:
        if( GetSkill( girl, SexType ) < 20 )
            message += " didn't do much as she allowed the customer to fuck her pussy.";
        else if( GetSkill( girl, SexType ) < 40 )
            message += " fucked the customer back while their cock was embedded in her cunt.";
        else if( GetSkill( girl, SexType ) < 60 )
            message += " liked the feeling of having a cock buried in her cunt and fucked back as much as she got.";
        else if( GetSkill( girl, SexType ) < 80 )
            message += " fucked like a wild animal, cumming several times and ending with her and the customer covered in sweat.";
        else
            message += GetRandomSexString();
            
        //" fucked hard and came many times, so much so that the bed was soaked in their sweat and juices.";
        break;
        
    case SKILL_BEASTIALITY:
        if( g_Brothels.GetNumBeasts() == 0 )
        {
            message += " found that there were no beasts available, so some fake ones were used. This disapointed the customer somewhat.";
            customer->m_Stats[STAT_HAPPINESS] -= 10;
        }
        else
        {
            if( GetSkill( girl, SexType ) < 50 )
            {
                // the less skilled she is, the more chance of hurting a beast accidentally
                int harmchance = -( GetSkill( girl, SexType ) - 50 ); // 50% chance at 0 skill, 1% chance at 49 skill
                
                if( g_Dice.percent( harmchance ) )
                {
                    message += " accidentally harmed some beasts during the act and she";
                    g_Brothels.add_to_beasts( -( ( g_Dice % 3 ) + 1 ) );
                }
            }
            
            if( GetSkill( girl, SexType ) < 20 )
                message += " was disgusted by the idea but still allowed the customer to watch as she was fucked by some animals.";
            else if( GetSkill( girl, SexType ) < 40 )
                message += " was a only little put off by the idea but still allowed the customer to watch and help as she was fucked by animals.";
            else if( GetSkill( girl, SexType ) < 60 )
                message += " took a large animal's cock deep inside her and enjoyed being fucked by it, her cries of pleasure being muffled by the customer's cock in her mouth.";
            else if( GetSkill( girl, SexType ) < 80 )
                message += " fucked some exotic beasts covered with massive cocks and tentacles, she came over and over alongside with the customer.";
            else
                message += GetRandomBeastString();
                
            //" came many times as she enjoyed all the pleasures of sex with animals and monsters, covered in cum she lay with the customer exhausted a long time.";
        }
        
        break;
        
    case SKILL_GROUP:
        if( GetSkill( girl, SexType ) < 20 )
            message += " struggled to service everyone in the group that came to fuck her.";
        else if( GetSkill( girl, SexType ) < 40 )
            message += " managed to keep the group of customers fucking her satisfied.";
        else if( GetSkill( girl, SexType ) < 60 )
            message += " serviced all of the group of customers that fucked her.";
        else if( GetSkill( girl, SexType ) < 80 )
            message += " fucked and came many times with everyone in the group of customers.";
        else
            message += GetRandomGroupString();
            
        //"'s orgasms could be heard through the building, along with all the customers in the group.";
        break;
        
    case SKILL_LESBIAN:
        if( GetSkill( girl, SexType ) < 20 )
            message += " licked her female customer's cunt until she came. She didn't want any herself.";
        else if( GetSkill( girl, SexType ) < 40 )
            message += " was aroused as she made her female customer cum.";
        else if( GetSkill( girl, SexType ) < 60 )
            message += " fucked and was fucked by her female customer.";
        else if( GetSkill( girl, SexType ) < 80 )
            message += " and her female customer's cumming could be heard thoughout the building.";
        else
            message += GetRandomLesString();
            
        //" came many times with her female customer, soaking the room in their juices.";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    // WD:  customer HAPPINESS changes complete now cap the stat to 100
    customer->m_Stats[STAT_HAPPINESS] = std::min( 100, ( int )customer->m_Stats[STAT_HAPPINESS] );
    
    if( SexType == SKILL_GROUP )
        message += "\nThe customers ";
    else
        message += "\nThe customer ";
        
    if( customer->m_Stats[STAT_HAPPINESS] > 80 )
        message += "swore they would come back.";
    else if( customer->m_Stats[STAT_HAPPINESS] > 50 )
        message += "enjoyed the experience.";
    else if( customer->m_Stats[STAT_HAPPINESS] > 30 )
        message += "didn't enjoy it.";
    else
        message += "thought it was crap.";
        
#if 1
    // WD: update Fame based on Customer HAPPINESS
    UpdateStat( girl, STAT_FAME, ( customer->m_Stats[STAT_HAPPINESS] - 1 ) / 33 );
#else
    
    // If he is happy then increase her fame
    if( customer->m_Stats[STAT_HAPPINESS] > 50 )
        UpdateStat( girl, STAT_FAME, ( ( ( ( int )customer->m_Stats[STAT_HAPPINESS] ) / 10 ) + 1 ) );
    
#endif
    
    // The girls STAT_CONSTITUTION and STAT_AGILITY modify how tired she gets
    AddTiredness( girl );
    
    // WD:  Allready being done in AddTiredness() Fn
    
    //if(g_Girls.GetStat(girl, STAT_TIREDNESS) == 100)
    //{
    //  UpdateStat(girl, STAT_HAPPINESS, -5);
    //  UpdateStat(girl, STAT_HEALTH, -5);
    //}
    
    // if the girl likes sex and the sex type then increase her happiness otherwise decrease it
    if( GetStat( girl, STAT_LIBIDO ) > 5 )
    {
        if( GetSkill( girl, SexType ) < 20 )
            message += "\nThough she had a tough time with it, she was horny and still managed to gain some little enjoyment.";
        else if( GetSkill( girl, SexType ) < 40 )
            message += "\nShe considered it a learning experience and enjoyed it a bit.";
        else if( GetSkill( girl, SexType ) < 60 )
            message += "\nShe enjoyed it a lot and wanted more.";
        else if( GetSkill( girl, SexType ) < 80 )
            message += "\nIt was nothing new for her, but she really does appreciate such work.";
        else
            message += "\nIt seems that she lives for this sort of thing.";
            
        UpdateStat( girl, STAT_HAPPINESS, GetStat( girl, STAT_LIBIDO ) / 5 );
    }
    else
    {
        message += "\nShe wasn't really in the mood.";
        UpdateStat( girl, STAT_HAPPINESS, -2 );
    }
    
    // special cases for certain sex types
    switch( SexType )
    {
    case SKILL_ANAL:
        if( GetSkill( girl, SexType ) <= 20 ) // if unexperienced then will get hurt
        {
            message += "\nHer inexperience hurt her a little.";
            UpdateStat( girl, STAT_HAPPINESS, -3 );
            UpdateStat( girl, STAT_CONFIDENCE, -1 );
            UpdateStat( girl, STAT_SPIRIT, -3 );
            UpdateStat( girl, STAT_HEALTH, -3 );
        }
        
        UpdateStat( girl, STAT_SPIRIT, -1 );
        
        break;
        
    case SKILL_BDSM:
        if( GetSkill( girl, SexType ) <= 30 ) // if unexperienced then will get hurt
        {
            message += "\nHer inexperience hurt her a little.";
            UpdateStat( girl, STAT_HAPPINESS, -2 );
            UpdateStat( girl, STAT_SPIRIT, -3 );
            UpdateStat( girl, STAT_CONFIDENCE, -1 );
            UpdateStat( girl, STAT_HEALTH, -3 );
        }
        
        contraception = girl->calc_pregnancy( customer, false, 0.75 );
        UpdateStat( girl, STAT_SPIRIT, -1 );
        break;
        
    case SKILL_NORMALSEX:
        if( GetSkill( girl, SexType ) < 10 )
        {
            message += "\nHer inexperience hurt her a little.";
            UpdateStat( girl, STAT_HAPPINESS, -2 );
            UpdateStat( girl, STAT_SPIRIT, -3 );
            UpdateStat( girl, STAT_CONFIDENCE, -1 );
            UpdateStat( girl, STAT_HEALTH, -3 );
        }
        
        /*
         *      if they're both happy afterward, it's good sex
         *      which modifies the chance of pregnancy
         */
        good = ( customer->happiness() >= 60 && girl->happiness() >= 60 );
        contraception = girl->calc_pregnancy( customer, good );
        break;
        
    case SKILL_BEASTIALITY:
        if( GetSkill( girl, SexType ) <= 30 ) // if unexperienced then will get hurt
        {
            message += "\nHer inexperience hurt her a little.";
            UpdateStat( girl, STAT_HAPPINESS, -2 );
            UpdateStat( girl, STAT_SPIRIT, -3 );
            UpdateStat( girl, STAT_CONFIDENCE, -1 );
            UpdateStat( girl, STAT_HEALTH, -3 );
        }
        
        UpdateStat( girl, STAT_SPIRIT, -1 ); // is pretty degrading
        /*
         *      if they're both happy afterward, it's good sex
         *      which modifies the chance of pregnancy
         */
        good = ( customer->happiness() >= 60 && girl->happiness() >= 60 );
        
        // mod: added check for number of beasts owned; otherwise, fake beasts could somehow inseminate the girl
        if( g_Brothels.GetNumBeasts() > 0 )
            contraception = girl->calc_insemination( customer, good );
            
        break;
        
    case SKILL_GROUP:
        if( GetSkill( girl, SexType ) <= 30 ) // if unexperienced then will get hurt
        {
            message += "\nHer inexperience hurt her a little.";
            UpdateStat( girl, STAT_HAPPINESS, -2 );
            UpdateStat( girl, STAT_SPIRIT, -3 );
            UpdateStat( girl, STAT_CONFIDENCE, -1 );
            UpdateStat( girl, STAT_HEALTH, -3 );
        }
        
        /*
         *      if they're both happy afterward, it's good sex
         *      which modifies the chance of pregnancy
         */
        good = ( customer->happiness() >= 60 && girl->happiness() >= 60 );
        /*
         *      adding a 50% bonus to the chance of pregnancy
         *      since there's more than one partner involved
         */
        contraception = girl->calc_pregnancy( customer, good, 1.5 );
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    // lose virginity unless it was anal sex
    if( girl->m_Virgin && SexType != SKILL_ANAL )
    {
        message += " The customer was overjoyed that she was a virgin.";
        girl->m_Virgin = false;
        customer->m_Stats[STAT_HAPPINESS] = 100;
    }
    
    // Now calculate other skill increases
    if( HasTrait( girl, "Quick Learner" ) )
    {
        if( SexType == SKILL_GROUP )
        {
            for( u_int i = 0; i < NUM_SKILLS; i++ )
                UpdateSkill( girl, i, g_Dice % 3 );
        }
        else
            UpdateSkill( girl, SexType, g_Dice % 5 );
            
        UpdateSkill( girl, SKILL_SERVICE, g_Dice % 5 );
        UpdateStat( girl, STAT_EXP, ( ( g_Dice % 7 ) * 3 ) ); // MYR: More xp, so levelling happens more
    }
    else if( HasTrait( girl, "Slow Learner" ) )
    {
        if( SexType == SKILL_GROUP )
        {
            for( u_int i = 0; i < NUM_SKILLS; i++ )
                UpdateSkill( girl, i, g_Dice % 2 );
        }
        else
            UpdateSkill( girl, SexType, g_Dice % 2 );
            
        UpdateSkill( girl, SKILL_SERVICE, g_Dice % 2 );
        UpdateStat( girl, STAT_EXP, ( ( g_Dice % 3 ) * 3 ) );
    }
    else
    {
        if( SexType == SKILL_GROUP )
        {
            for( u_int i = 0; i < NUM_SKILLS; i++ )
                UpdateSkill( girl, i, g_Dice % 2 );
        }
        else
            UpdateSkill( girl, SexType, g_Dice % 3 );
            
        UpdateSkill( girl, SKILL_SERVICE, g_Dice % 3 );
        UpdateStat( girl, STAT_EXP, ( ( g_Dice % 5 ) * 3 ) );
    }
    
    if( GetStat( girl, STAT_HAPPINESS ) > 50 && !HasTrait( girl, "Nymphomaniac" ) )
    {
        g_Girls.UpdateEnjoyment( girl, ACTION_SEX, +1, true );
        //UpdateStat(girl,STAT_LIBIDO,-3);  // libido is satisfied after sex  // Libido decremented in JobManager
    }
    else
    {
        g_Girls.UpdateEnjoyment( girl, ACTION_SEX, +2, true );
    }
    
    if( GetStat( girl, STAT_HAPPINESS ) <= 5 )
    {
        g_Girls.UpdateEnjoyment( girl, ACTION_SEX, -1, true );
    }
    
    //UpdateStat(girl,STAT_LIBIDO,-1);  // Libido decremented in JobManager
    
    if( HasTrait( girl, "AIDS" ) )
        customer->m_Stats[STAT_HAPPINESS] -= 10;
    else if( GetStat( girl, STAT_HEALTH ) <= 10 )
    {
        if( ( g_Dice % 100 ) + 1 == 1 && !contraception )
        {
            std::string mess = "";
            mess += girl->m_Realname;
            mess += " has caught the disease AIDS! She will likely die, but a rare cure can sometimes be found in the shop.";
            girl->m_Events.AddMessage( mess, IMGTYPE_PROFILE, EVENT_DANGER );
            AddTrait( girl, "AIDS" );
        }
    }
    
    if( HasTrait( girl, "Chlamydia" ) )
        customer->m_Stats[STAT_HAPPINESS] -= 20;
    else if( GetStat( girl, STAT_HEALTH ) <= 10 )
    {
        if( ( g_Dice % 100 ) + 1 <= 1 && !contraception )
        {
            std::string mess = "";
            mess += girl->m_Realname;
            mess += " has caught the disease Chlamydia! A cure can sometimes be found in the shop.";
            girl->m_Events.AddMessage( mess, IMGTYPE_PROFILE, EVENT_DANGER );
            AddTrait( girl, "Chlamydia" );
        }
    }
    
    if( HasTrait( girl, "Syphilis" ) )
        customer->m_Stats[STAT_HAPPINESS] -= 10;
    else if( GetStat( girl, STAT_HEALTH ) <= 10 )
    {
        if( ( g_Dice % 100 ) + 1 <= 1 && !contraception )
        {
            std::string mess = "";
            mess += girl->m_Realname;
            mess += " has caught the disease Syphilis! This can be deadly, but a cure can sometimes be found in the shop.";
            girl->m_Events.AddMessage( mess, IMGTYPE_PROFILE, EVENT_DANGER );
            AddTrait( girl, "Syphilis" );
        }
    }
}

std::string cGirls::GetRandomSexString()
{
    int roll1 = 0, roll2 = 0, roll3 = 0, random = 0;
    std::string OutStr;
    
    
    // MYR: Can't resist a little cheeky chaos
    random = g_Dice % 500;
    
    if( random == 345 )
    {
        OutStr += " (phrase 1). (phrase 2) (phrase 3).";
        return OutStr;
    }
    
    OutStr += " ";  // Consistency
    
    // Roll #1
    # pragma region sex1
    roll1 = g_Dice % 8 + 1;   // Remember to update this when new strings are added
    
    switch( roll1 )
    {
    case 1:
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "straddled";
        else if( random <= 4 )
            OutStr += "cow-girled";
        else if( random <= 6 )
            OutStr += "wrapped her legs around";
        else if( random <= 8 )
            OutStr += "contorted her legs behind her head for";
        else
            OutStr += "scissored";
            
        OutStr += " the client, because it ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "turned him on";
        else if( random <= 4 )
            OutStr += "made him crazy";
        else if( random <= 6 )
            OutStr += "gave him a massive boner";
        else if( random <= 8 )
            OutStr += "was more fun than talking";
        else
            OutStr += "made him turn red";
            
        break;
        
    case 2:
        OutStr += "was told to grab ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "her ankles";
        else if( random <= 4 )
            OutStr += "the chair";
        else if( random <= 6 )
            OutStr += "her knees";
        else if( random <= 8 )
            OutStr += "the table";
        else
            OutStr += "the railing";
            
        OutStr += " and ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "shook her hips";
        else if( random <= 4 )
            OutStr += "spread her legs";
        else if( random <= 6 )
            OutStr += "close her eyes";
        else if( random <= 8 )
            OutStr += "look away";
        else
            OutStr += "bend waaaaayyy over";
            
        break;
        
    case 3:
        OutStr += "had fun with his ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "foot";
        else if( random <= 4 )
            OutStr += "stocking";
        else if( random <= 6 )
            OutStr += "hair";
        else if( random <= 8 )
            OutStr += "lace";
        else if( random <= 10 )
            OutStr += "butt";
        else
            OutStr += "food";
            
        OutStr += " fetish and gave him an extended ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "foot";
        else if( random <= 4 )
            OutStr += "hand";
        else
            OutStr += "oral";
            
        OutStr += " surprise";
        
        break;
        
    case 4:
        OutStr += "dressed as ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "a school girl";
        else if( random <= 4 )
            OutStr += "a nurse";
        else if( random <= 6 )
            OutStr += "a nun";
        else if( random <= 8 )
            OutStr += "an adventurer";
        else
            OutStr += "a dominatrix";
            
        OutStr += " to grease ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "the little man";
        else if( random <= 4 )
            OutStr += "his pole";
        else if( random <= 6 )
            OutStr += "his tool";
        else if( random <= 8 )
            OutStr += "his fingers";
        else
            OutStr += "his toes";
            
        break;
        
    case 5:
        OutStr += "decided to skip ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "the bed";
        else if( random <= 4 )
            OutStr += "foreplay";
        else if( random <= 6 )
            OutStr += "niceties";
        else
            OutStr += "greetings";
            
        OutStr += " and assumed position ";
        
        random = g_Dice % 9999 + 1;
        /*char buffer[10];*/
        OutStr += toString( random );
        
        break;
        
    case 6:
        OutStr += "gazed in awe at ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "how well hung he was";
        else if( random <= 4 )
            OutStr += "the time";
        else if( random <= 6 )
            OutStr += "his muscles";
        else if( random <= 8 )
            OutStr += "his handsome face";
        else
            OutStr += "his collection of sexual magic items";
            
        OutStr += " and ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "felt inspired";
        else if( random <= 4 )
            OutStr += "played hard to get";
        else if( random <= 6 )
            OutStr += "squealed like a little girl";
        else
            OutStr += "prepared for action";
            
        break;
        
    case 7:
        OutStr += "bent into ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "a delightful";
        else if( random <= 4 )
            OutStr += "an awkward";
        else if( random <= 6 )
            OutStr += "a difficult";
        else
            OutStr += "a crazy";
            
        OutStr += " position and ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "squealed";
        else if( random <= 4 )
            OutStr += "moaned";
        else
            OutStr += "grew hot";
            
        OutStr += " as he ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "touched";
        else if( random <= 4 )
            OutStr += "caressed";
        else
            OutStr += "probed";
            
        OutStr += " her defenseless body";
        break;
        
    case 8:
        OutStr += "lay on the ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "floor";
        else if( random <= 4 )
            OutStr += "bed";
        else
            OutStr += "couch";
            
        OutStr += " and ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "had him take off all her clothes";
        else if( random <= 4 )
            OutStr += "told him exactly what turned her on";
        else
            OutStr += "encouraged him to take off her bra and panties with his teeth";
            
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion sex1
    
    // Roll #2
    # pragma region sex2
    OutStr += ". ";
    
    roll2 = g_Dice % 11 + 1;
    
    switch( roll2 )
    {
    case 1:
        OutStr += "She ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "rode him all the way to the next town";
        else if( random <= 4 )
            OutStr += "massaged his balls and sucked him dry";
        else if( random <= 6 )
            OutStr += "titty fucked and sucked the well dry";
        else
            OutStr += "fucked him blind";
            
        OutStr += ". He was a trooper though and rallied: She ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "was deeply penetrated";
        else if( random <= 4 )
            OutStr += "was paralyzed with stunning sensations";
        else if( random <= 6 )
            OutStr += "bucked like a bronko";
        else
            OutStr += "shook with pleasure";
            
        OutStr += " and ";
        
        random = g_Dice % 4 + 1;
        
        if( random <= 2 )
            OutStr += "came like a fire hose from";
        else
            OutStr += "repeatedly shook in orgasm with";
            
        break;
        
    case 2:
        OutStr += "It took a lot of effort to stay ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 3 )
            OutStr += "interested in";
        else if( random <= 7 )
            OutStr += "awake for";
        else
            OutStr += "conscious for";
            
        break;
        
    case 3:
        OutStr += "She was fucked ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "blind";
        else if( random <= 4 )
            OutStr += "silly twice over";
        else if( random <= 6 )
            OutStr += "all crazy like";
        else if( random <= 8 )
            OutStr += "for hours";
        else
            OutStr += "for minutes";
            
        OutStr += " by";
        break;
        
    case 4:
        OutStr += "She performed ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "uninspired ";
        else
            OutStr += "inspired ";
            
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "missionary ";
        else if( random <= 4 )
            OutStr += "oral ";
        else if( random <= 6 )
            OutStr += "foot ";
        else
            OutStr += "hand ";
            
        OutStr += "sex for";
        break;
        
    case 5:
        //OutStr += "";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "Semen";
        else if( random <= 4 )
            OutStr += "Praise";
        else if( random <= 6 )
            OutStr += "Flesh";
        else if( random <= 8 )
            OutStr += "Drool";
        else
            OutStr += "Chocolate sauce";
            
        OutStr += " rained down on her from";
        break;
        
    case 6:
        OutStr += "She couldn't ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "stand";
        else if( random <= 4 )
            OutStr += "walk";
        else if( random <= 6 )
            OutStr += "swallow";
        else if( random <= 8 )
            OutStr += "feel her legs";
        else
            OutStr += "move";
            
        OutStr += " after screwing";
        break;
        
    case 7:
        OutStr += "It took a great deal of effort to look ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "interested in";
        else if( random <= 4 )
            OutStr += "awake for";
        else if( random <= 6 )
            OutStr += "alive for";
        else if( random <= 8 )
            OutStr += "enthusiastic for";
        else
            OutStr += "hurt for";
            
        break;
        
    case 8:
        OutStr += "She played 'clean up the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "tools";
        else if( random <= 4 )
            OutStr += "customer";
        else if( random <= 6 )
            OutStr += "sword";
        else
            OutStr += "sugar frosting";
            
        OutStr += "' with";
        break;
        
    case 9:
        OutStr += "Hopefully her ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "cervix";
        else if( random <= 4 )
            OutStr += "pride";
        else if( random <= 6 )
            OutStr += "reputation";
        else if( random <= 8 )
            OutStr += "ego";
        else
            OutStr += "stomach";
            
        OutStr += " wasn't bruised by";
        break;
        
    case 10:
        OutStr += "She called in ";
        
        random = g_Dice % 3 + 2;
        /*char buffer[10];*/
        OutStr += toString( random );
        
        OutStr += " reinforcements to tame";
        break;
        
    case 11:
        OutStr += "She orgasmed ";
        
        random = g_Dice % 100 + 30;
        OutStr += toString( random );
        
        OutStr += " times with";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion sex2
    
    // Roll #3
    # pragma region sex3
    OutStr += " ";  // Consistency
    
    roll3 = g_Dice % 20 + 1;
    
    switch( roll3 )
    {
    case 1:
        OutStr += "the guy ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "wearing three amulets of the sex elemental.";
        else if( random <= 4 )
            OutStr += "wearing eight rings of the horndog.";
        else if( random <= 6 )
            OutStr += "wearing a band of invulnerability.";
        else if( random <= 8 )
            OutStr += "carrying a waffle iron.";
        else
            OutStr += "carrying a body probe of irresistable sensations.";
            
        break;
        
    case 2:
        OutStr += "Thor, God of Thunderfucking!!!!";
        break;
        
    case 3:
        OutStr += "the frustrated ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "astronomer.";
        else if( random <= 4 )
            OutStr += "physicist.";
        else if( random <= 6 )
            OutStr += "chemist.";
        else if( random <= 8 )
            OutStr += "biologist.";
        else
            OutStr += "engineer.";
            
        break;
        
    case 4:
        OutStr += "the invisible something or other????";
        break;
        
    case 5:
        OutStr += "the butler. (He always did it.)";
        break;
        
    case 6:
        OutStr += "the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "sentient apple tree.";
        else if( random <= 4 )
            OutStr += "sentient sex toy.";
        else if( random <= 6 )
            OutStr += "pan-dimensional toothbrush.";
        else if( random <= 8 )
            OutStr += "magic motorcycle.";
        else
            OutStr += "regular bloke.";
            
        break;
        
    case 7:
        OutStr += "the unbelievably well behaved ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "Pink Petal forum member.";
        else if( random <= 4 )
            OutStr += "tentacle.";
        else if( random <= 6 )
            OutStr += "pirate.";
        else
            OutStr += "sentient bottle.";
            
        break;
        
    case 8:
        random = g_Dice % 20 + 1;
        
        if( random <= 2 )
            OutStr += "Cousin";
        else if( random <= 4 )
            OutStr += "Brother";
        else if( random <= 6 )
            OutStr += "Saint";
        else if( random <= 8 )
            OutStr += "Lieutenant";
        else if( random <= 10 )
            OutStr += "Master";
        else if( random <= 12 )
            OutStr += "Doctor";
        else if( random <= 14 )
            OutStr += "Mr.";
        else if( random <= 16 )
            OutStr += "Smith";
        else if( random <= 18 )
            OutStr += "DockMaster";
        else
            OutStr += "Perfect";
            
        OutStr += " Parkins from down the street.";
        break;
        
    case 9:
        OutStr += "the master of the hidden dick technique. (Where is it? Nobody knows.)";
        break;
        
    case 10:
        OutStr += "cake. It isn't a lie!";
        break;
        
    case 11:
        OutStr += "the really, really macho ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "Titan.";
        else if( random <= 4 )
            OutStr += "Storm Giant.";
        else if( random <= 6 )
            OutStr += "small moon.";
        else if( random <= 8 )
            OutStr += "kobold.";
        else
            OutStr += "madness.";
            
        break;
        
    case 12:
        OutStr += "the clockwork man!";
        
        OutStr += " (With no sensation in his clockwork ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "tool";
        else if( random <= 4 )
            OutStr += "head";
        else if( random <= 6 )
            OutStr += "fingers";
        else if( random <= 8 )
            OutStr += "attachment";
        else
            OutStr += "clock";
            
        OutStr += " and no sense to ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "stop";
        else if( random <= 4 )
            OutStr += "slow down";
        else if( random <= 6 )
            OutStr += "moderate";
        else if( random <= 8 )
            OutStr += "be gentle";
        else
        {
            OutStr += "stop at ";
            
            random = g_Dice % 50 + 30;
            OutStr += toString( random );
            
            OutStr += " orgasms";
        }
        
        OutStr += ".)";
        break;
        
    case 13:
        // MYR: This one gives useful advice to the players.  A gift from us to them.
        OutStr += "the Brothel Master developer. ";
        
        random = g_Dice % 20 + 1;
        
        if( random <= 2 )
            OutStr += "(Quick learner is a great talent to have.)";
        else if( random <= 4 )
            OutStr += "(Don't ignore the practice skills option for your girls.)";
        else if( random <= 6 )
            OutStr += "(Train your gangs.)";
        else if( random <= 8 )
            OutStr += "(Every time you restart the game, the shop inventory is reset.)";
        else if( random <= 10 )
            OutStr += "(Invulnerable (insubstantial) characters should be exploring the catacombs.)";
        else if( random <= 12 )
            OutStr += "(High dodge gear is great for characters exploring the catacombs.)";
        else if( random <= 14 )
            OutStr += "(For a character with a high constitution, experiment with working on both shifts.)";
        else if( random <= 16 )
            OutStr += "(Matrons need high service skills.)";
        else if( random <= 18 )
            OutStr += "(Girls see a max of 3 people for high reputations, 3 for high appearance and 3 for high skills.)";
        else
            OutStr += "(Don't overlook the bribery option in the town hall and the bank.)";
            
        break;
        
    case 14:
        OutStr += "grandmaster piledriver the 17th.";
        break;
        
    case 15:
        OutStr += "the evolved sexual entity from ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
        {
            random = g_Dice % 200000 + 100000;
            OutStr += toString( random );
            OutStr += " years in the future.";
        }
        else if( random <= 4 )
            OutStr += "the closet.";
        else if( random <= 6 )
            OutStr += "the suburbs.";
        else
            OutStr += "somewhere in deep space.";
            
        break;
        
    case 16:
        OutStr += "the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "mayor";
        else if( random <= 4 )
            OutStr += "bishop";
        else if( random <= 6 )
            OutStr += "town treasurer";
        else
            OutStr += "school principle";
            
        OutStr += ", on one of his regular health checkups.";
        break;
        
    case 17:
        OutStr += "the letter H.";
        break;
        
    case 18:
        OutStr += "a completely regular and unspectacular guy.";
        break;
        
    case 19:
        OutStr += "the ";
        
        random = g_Dice % 20 + 5;
        OutStr += toString( random );
        OutStr += " dick, ";
        
        random = g_Dice % 20 + 5;
        OutStr += toString( random );
        OutStr += "-armed ";
        
        OutStr += "(Each wearing ";
        
        random = g_Dice % 2 + 4;
        OutStr += toString( random );
        OutStr += " ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "rings of the Schwarzenegger";
        else if( random <= 4 )
            OutStr += "rings of the horndog";
        else if( random <= 6 )
            OutStr += "rings of beauty";
        else
            OutStr += "rings of potent sexual stamina";
            
        OutStr += ") ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "neighbor";
        else if( random <= 4 )
            OutStr += "yugoloth";
        else if( random <= 6 )
            OutStr += "abberation";
        else
            OutStr += "ancient one";
            
        OutStr += ".";
        break;
        
    case 20:
        OutStr += "the number 69.";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion sex3
    
    OutStr += "\n";
    return OutStr;
}

std::string cGirls::GetRandomGroupString()
{
    int roll1 = 0, roll2 = 0, roll3 = 0, random = 0;
    std::string OutStr;
    /*char buffer[10];*/
    
    // Part 1
    # pragma region group1
    OutStr += " ";
    
    roll1 = g_Dice % 4 + 1;   // Remember to update this when new strings are added
    
    switch( roll1 )
    {
    case 1:
        OutStr += "counted the number of customers: ";
        
        random = g_Dice % 20 + 5;
        OutStr += toString( random );
        OutStr += ". ";
        
        random = g_Dice % 14 + 1;
        
        if( random <= 2 )
            OutStr += "This was going to be rough";
        else if( random <= 4 )
            OutStr += "Sweet";
        else if( random <= 6 )
            OutStr += "It could be worse";
        else if( random <= 8 )
            OutStr += "A smile formed on her lips. This was going to be fun";
        else if( random <= 10 )
            OutStr += "Boring";
        else if( random <= 12 )
            OutStr += "Not enough";
        else
            OutStr += "'Could you get more?' she wondered";
            
        break;
        
    case 2:
        OutStr += "was lost in ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "a sea";
        else if( random <= 4 )
            OutStr += "a storm";
        else if( random <= 6 )
            OutStr += "an ocean";
        else
            OutStr += "a jungle";
            
        OutStr += " of hot bodies";
        break;
        
    case 3:
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "sat";
        else if( random <= 4 )
            OutStr += "lay";
        else if( random <= 6 )
            OutStr += "stood";
        else
            OutStr += "crouched";
            
        OutStr += " ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "blindfolded and ";
            
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "handcuffed";
        else if( random <= 4 )
            OutStr += "tied up";
        else if( random <= 6 )
            OutStr += "wrists bound in rope";
        else
            OutStr += "wrists in chains hanging from the ceiling";
            
        OutStr += " in the middle of a ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "circle";
        else if( random <= 4 )
            OutStr += "smouldering pile";
        else if( random <= 6 )
            OutStr += "phalanx";
        else
            OutStr += "wall";
            
        OutStr += " of flesh";
        break;
        
    case 4:
        OutStr += "was ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "amazed by";
        else if( random <= 4 )
            OutStr += "disappointed by";
        else if( random <= 6 )
            OutStr += "overjoyed with";
        else
            OutStr += "ecstatically happy with";
            
        OutStr += " the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "brigade";
        else if( random <= 4 )
            OutStr += "army group";
        else if( random <= 6 )
            OutStr += "squad";
        else
            OutStr += "batallion";
            
        OutStr += " of ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "man meat";
        else if( random <= 4 )
            OutStr += "cock";
        else if( random <= 6 )
            OutStr += "muscle";
        else
            OutStr += "horny, brainless thugs";
            
        OutStr += " around her";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion group1
    
    // Part 2
    # pragma region group2
    OutStr += ". ";
    
    roll2 = g_Dice % 8 + 1;
    
    switch( roll2 )
    {
    case 1:
        OutStr += "She was thoroughly ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "banged";
        else if( random <= 4 )
            OutStr += "fucked";
        else if( random <= 6 )
            OutStr += "disappointed";
        else
            OutStr += "penetrated";
            
        OutStr += " by";
        break;
        
    case 2:
        OutStr += "They handled her like ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "an expensive";
        else
            OutStr += "a cheap";
            
        OutStr += " ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "hooker";
        else if( random <= 4 )
            OutStr += "street worker";
        else if( random <= 6 )
            OutStr += "violin";
        else
            OutStr += "wine";
            
        OutStr += " for";
        break;
        
    case 3:
        OutStr += "Her ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "holes were";
        else
            OutStr += "love canal was";
            
        OutStr += " plugged by";
        break;
        
    case 4:
        OutStr += "She ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "was bukkaked by";
        else if( random <= 4 )
            OutStr += "was given pearl necklaces by";
        else
            OutStr += "received a thorough face/hair job from";
            
        break;
        
    case 5:
        OutStr += "They demanded simultaneous hand, foot and mouth ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "jobs";
        else if( random <= 4 )
            OutStr += "action";
        else
            OutStr += "combat";
            
        OutStr += " for";
        break;
        
    case 6:
        OutStr += "There was a positive side: 'So much ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "meat";
        else if( random <= 4 )
            OutStr += "cock";
        else if( random <= 6 )
            OutStr += "testosterone";
        else
            OutStr += "to do";
            
        OutStr += ", so little time' she said to";
        break;
        
    case 7:
        OutStr += "They made sure she had a nutritious meal of ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "man meat";
        else if( random <= 4 )
            OutStr += "cock";
        else if( random <= 6 )
            OutStr += "penis";
        else
            OutStr += "meat rods";
            
        OutStr += " and drinks of delicious ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "semen";
        else if( random <= 4 )
            OutStr += "man mucus";
        else if( random <= 6 )
            OutStr += "man-love";
        else
            OutStr += "man-cream";
            
        OutStr += " from";
        break;
        
    case 8:
        OutStr += "She was ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "fucked";
        else if( random <= 4 )
            OutStr += "banged";
        else if( random <= 6 )
            OutStr += "humped";
        else
            OutStr += "sucked";
            
        OutStr += " silly ";
        
        if( random <= 2 )
            OutStr += "twice over";
        else if( random <= 4 )
            OutStr += "three times over";
        else
            OutStr += "so many times";
            
        OutStr += " by";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion group2
    
    // Part 3
    # pragma region group3
    OutStr += " ";
    
    roll3 = g_Dice % 11 + 1;
    
    switch( roll3 )
    {
    case 1:
        OutStr += "every member of the Crossgate ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "roads crew.";
        else if( random <= 4 )
            OutStr += "administrative staff.";
        else if( random <= 6 )
            OutStr += "interleague volleyball team.";
        else
            OutStr += "short persons defense league.";
            
        ;
        break;
        
    case 2:
        OutStr += "all the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "lieutenants";
        else if( random <= 4 )
            OutStr += "sergeants";
        else if( random <= 6 )
            OutStr += "captains";
        else
            OutStr += "junior officers";
            
        OutStr += " in the Mundigan ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "army.";
        else if( random <= 4 )
            OutStr += "navy.";
        else if( random <= 6 )
            OutStr += "elite forces.";
        else
            OutStr += "foreign legion.";
            
        break;
        
    case 3:
        OutStr += "the visiting ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "half-giants. (Ouch!)";
        else if( random <= 4 )
            OutStr += "storm giants.";
        else if( random <= 6 )
            OutStr += "titans.";
        else
            OutStr += "ogres.";
            
        break;
        
    case 4:
        OutStr += "the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "Hentai Research";
        else if( random <= 4 )
            OutStr += "Women's Rights";
        else if( random <= 6 )
            OutStr += "Prostitution Studies";
        else
            OutStr += "Celibacy";
            
        OutStr += " club of the University of Cunning Linguists.";
        break;
        
    case 5:
        OutStr += "the squad of ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "hard-to-find ninjas.";
        else
            OutStr += "racous pirates.";
            
        break;
        
    case 6:
        OutStr += "a group of people from some place called the 'Pink Petal forums'.";
        break;
        
    case 7:
        OutStr += "the seemingly endless ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "army";
        else if( random <= 4 )
            OutStr += "horde";
        else if( random <= 6 )
            OutStr += "number";
        else
            OutStr += "group";
            
        OutStr += " of really";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += ", really ";
        else
            OutStr += " ";
            
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "fired up";
        else if( random <= 4 )
            OutStr += "horny";
        else if( random <= 6 )
            OutStr += "randy";
        else
            OutStr += "backed up";
            
        OutStr += " ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "gnomes.";
        else if( random <= 4 )
            OutStr += "halflings.";
        else if( random <= 6 )
            OutStr += "kobolds.";
        else
            OutStr += "office workers.";
            
        break;
        
    case 8:
        OutStr += "CSI ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "New York";
        else if( random <= 4 )
            OutStr += "Miami";
        else if( random <= 6 )
            OutStr += "Mundigan";
        else
            OutStr += "Tokyo";
            
        OutStr += " branch.";
        break;
        
    case 9:
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "frosh";
        else if( random <= 4 )
            OutStr += "seniors";
        else if( random <= 6 )
            OutStr += "young adults";
        else
            OutStr += "women";
            
        OutStr += " on a ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "serious ";
            
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "bender";
        else if( random <= 4 )
            OutStr += "road trip";
        else if( random <= 6 )
            OutStr += "medical study";
        else
            OutStr += "lark";
            
        OutStr += ".";
        break;
        
    case 10:
        OutStr += "all the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "second stringers";
        else if( random <= 4 )
            OutStr += "has-beens";
        else if( random <= 6 )
            OutStr += "never-weres";
        else
            OutStr += "victims";
            
        OutStr += " from the ";
        
        random = g_Dice % 20 + 1991;
        OutStr += toString( random );
        
        OutStr += " H anime season.";
        break;
        
    case 11:
        OutStr += "Grandpa Parkins and his extended family of ";
        
        random = g_Dice % 200 + 100;
        OutStr += toString( random );
        
        OutStr += ".";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += " (And ";
            
            random = g_Dice % 100 + 50;
            OutStr += toString( random );
            
            OutStr += " guests.)";
        }
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += " (And ";
            
            random = g_Dice % 100 + 50;
            OutStr += toString( random );
            
            OutStr += " more from the extended extended family.)";
        }
        
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion group3
    
    OutStr += "\n";
    return OutStr;
}

std::string cGirls::GetRandomBDSMString()
{
    int roll2 = 0, roll3 = 0, random = 0;
    std::string OutStr;
    /*char buffer[10];*/
    
    OutStr += " was ";
    
    // Part 1:
    # pragma region bdsm1
    // MYR: Was on a roll so I completely redid the first part
    
    random = g_Dice % 12 + 1;
    
    if( random <= 2 )
        OutStr += "dressed as a dominatrix";
    else if( random <= 4 )
        OutStr += "stripped naked";
    else if( random <= 6 )
        OutStr += "dressed as a (strictly legal age) school girl";
    else if( random <= 8 )
        OutStr += "dressed as a nurse";
    else if( random <= 10 )
        OutStr += "put in heels";
    else
        OutStr += "covered in oil";
        
    random = g_Dice % 4 + 1;
    
    if( random == 3 )
    {
        random = g_Dice % 4 + 1;
        
        if( random <= 2 )
            OutStr += ", rendered helpless by drugs";
        else
            OutStr += ", restrained by magic";
    }
    
    random = g_Dice % 4 + 1;
    
    if( random == 2 )
        OutStr += ", blindfolded";
        
    random = g_Dice % 4 + 1;
    
    if( random == 2 )
        OutStr += ", gagged";
        
    OutStr += ", and ";
    
    random = g_Dice % 12 + 1;
    
    if( random <= 2 )
        OutStr += "chained";
    else if( random <= 4 )
        OutStr += "lashed";
    else if( random <= 6 )
        OutStr += "tied";
    else if( random <= 8 )
        OutStr += "bound";
    else if( random <= 10 )
        OutStr += "cuffed";
    else
        OutStr += "leashed";
        
    random = g_Dice % 4 + 1;
    
    if( random == 3 )
        OutStr += ", arms behind her back";
        
    random = g_Dice % 4 + 1;
    
    if( random == 2 )
        OutStr += ", fettered";
        
    random = g_Dice % 4 + 1;
    
    if( random == 2 )
        OutStr += ", spread eagle";
        
    random = g_Dice % 4 + 1;
    
    if( random == 2 )
        OutStr += ", upside down";
        
    OutStr += " ";
    
    random = g_Dice % 16 + 1;
    
    if( random <= 2 )
        OutStr += "to a bed";
    else if( random <= 4 )
        OutStr += "to a post";
    else if( random <= 6 )
        OutStr += "to a wall";
    else if( random <= 8 )
        OutStr += "to vertical stocks";
    else if( random <= 10 )
        OutStr += "to a table";
    else if( random <= 12 )
        OutStr += "on a wooden horse";
    else if( random <= 14 )
        OutStr += "in stocks";
    else
        OutStr += "at the dog house";
        
    # pragma endregion bdsm1
    
    // Part 2
    # pragma region bdsm2
    OutStr += ". ";
    
    roll2 = g_Dice % 8 + 1;
    
    switch( roll2 )
    {
    case 1:
        OutStr += "She was fucked ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "with a rake";
        else if( random <= 4 )
            OutStr += "with a giant dildo";
        else if( random <= 6 )
            OutStr += "and flogged";
        else if( random <= 8 )
            OutStr += "and lashed";
        else if( random <= 10 )
            OutStr += "tenderly";
        else
            OutStr += "like a dog";
            
        OutStr += " by";
        break;
        
    case 2:
        OutStr += "Explanations were necessary before she was ";
        
        random = g_Dice % 14 + 1;
        
        if( random <= 2 )
            OutStr += "screwed";
        else if( random <= 4 )
            OutStr += "penetrated";
        else if( random <= 6 )
            OutStr += "abused";
        else if( random <= 8 )
            OutStr += "whipped";
        else if( random <= 10 )
            OutStr += "yelled at";
        else if( random <= 12 )
            OutStr += "banged repeatedly";
        else
            OutStr += "smacked around";
            
        OutStr += " by";
        break;
        
    case 3:
        OutStr += "Her holes were filled ";
        
        random = g_Dice % 16 + 1;
        
        if( random <= 2 )
            OutStr += "with wiggly things";
        else if( random <= 4 )
            OutStr += "with vibrating things";
        else if( random <= 6 )
            OutStr += "with sex toys";
        else if( random <= 8 )
            OutStr += "by things with uncomfortable edges";
        else if( random <= 10 )
            OutStr += "with marbles";
        else if( random <= 12 )
            OutStr += "with foreign objects";
        else if( random <= 14 )
            OutStr += "with hopes and dreams";
        else
            OutStr += "with semen";
            
        OutStr += " by";
        break;
        
    case 4:
        OutStr += "A massive aphrodisiac was administered before she was ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "teased";
        else if( random <= 4 )
            OutStr += "fucked";
        else if( random <= 6 )
            OutStr += "left alone";
        else if( random <= 8 )
            OutStr += "repeatedly brought to the edge of orgasm, but not over";
        else
            OutStr += "mercilessly tickled by a feather wielded";
            
        OutStr += " by";
        break;
        
    case 5:
        OutStr += "Entertainment was demanded before she was ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "humped";
        else if( random <= 4 )
            OutStr += "rough-housed";
        else if( random <= 6 )
            OutStr += "pinched over and over";
        else if( random <= 8 )
            OutStr += "probed by instruments";
        else if( random <= 10 )
            OutStr += "fondled roughly";
        else
            OutStr += "sent away";
            
        OutStr += " by";
        break;
        
    case 6:
        OutStr += "She was pierced repeatedly by ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "needles";
        else if( random <= 4 )
            OutStr += "magic missiles";
        else
            OutStr += "evil thoughts";
            
        OutStr += " from";
        break;
        
    case 7:
        //OutStr += "She had ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "Weights";
        else if( random <= 4 )
            OutStr += "Christmas ornaments";
        else if( random <= 6 )
            OutStr += "Lewd signs";
        else if( random <= 8 )
            OutStr += "Trinkets";
        else
            OutStr += "Abstract symbols";
            
        OutStr += " were hung from her unmentionables by";
        break;
        
    case 8:
        OutStr += "She was ordered to ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "flail herself";
        else if( random <= 4 )
            OutStr += "perform fellatio";
        else if( random <= 6 )
            OutStr += "masturbate";
        else
            OutStr += "beg for it";
            
        OutStr += " by";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion bdsm2
    
    // Part 3
    # pragma region bdsm3
    OutStr += " ";
    
    roll3 = g_Dice % 18 + 1;
    
    switch( roll3 )
    {
    case 1:
        OutStr += "Iron Man.";
        break;
        
    case 2:
        OutStr += "the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "wizard";
        else if( random <= 4 )
            OutStr += "sorceress";
        else if( random <= 6 )
            OutStr += "archmage";
        else
            OutStr += "warlock";
            
        OutStr += "'s ";
        
        random = g_Dice % 8;
        
        if( random <= 2 )
            OutStr += "golem.";
        else if( random <= 4 )
            OutStr += "familiar.";
        else if( random <= 6 )
            OutStr += "homoculous.";
        else
            OutStr += "summoned monster.";
            
        break;
        
    case 3:
        OutStr += "the amazingly hung ";
        
        random = g_Dice % 8;
        
        if( random <= 2 )
            OutStr += "goblin.";
        else if( random <= 4 )
            OutStr += "civic worker.";
        else if( random <= 6 )
            OutStr += "geletanious cube.";
        else
            OutStr += "sentient shirt.";   // MYR: I love this one.
            
        break;
        
    case 4:
        OutStr += "the pirate dressed as a ninja. (Cool things are cool.)";
        break;
        
    case 5:
        OutStr += "Hannibal Lecter.";
        break;
        
    case 6:
        OutStr += "the stoned ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "dark elf.";
        else if( random <= 4 )
            OutStr += "gargoyle.";
        else if( random <= 6 )
            OutStr += "earth elemental.";
        else if( random <= 8 )
            OutStr += "astral deva.";
        else
            OutStr += "college kid.";
            
        break;
        
    case 7:
        OutStr += "your hyperactive ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "cousin.";
        else if( random <= 4 )
            OutStr += "grandmother.";
        else if( random <= 6 )
            OutStr += "grandfather.";
        else if( random <= 8 )
            OutStr += "brother.";
        else
            OutStr += "sister.";
            
        break;
        
    case 8:
        OutStr += "someone who looks exactly like you!";
        break;
        
    case 9:
        OutStr += "the horny ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "genie.";
        else if( random <= 4 )
            OutStr += "fishmonger.";
        else if( random <= 6 )
            OutStr += "chauffeur.";
        else if( random <= 8 )
            OutStr += "Autobot.";
        else
            OutStr += "thought.";
            
        break;
        
    case 10:
        OutStr += "the rampaging ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "english professor.";
        else if( random <= 4 )
            OutStr += "peace activist.";
        else if( random <= 6 )
            OutStr += "color red.";
        else if( random <= 8 )
            OutStr += "special forces agent.";
        else
            OutStr += "chef.";
            
        break;
        
    case 11:
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "disloyal thugs";
        else if( random <= 4 )
            OutStr += "girls";
        else if( random <= 6 )
            OutStr += "dissatisfied customers";
        else if( random <= 8 )
            OutStr += "workers";
        else
            OutStr += "malicious agents";
            
        OutStr += " from a competing brothel.";
        break;
        
    case 12:
        OutStr += "a cruel ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "Cyberman.";
        else if( random <= 4 )
            OutStr += "Dalek.";
        else if( random <= 6 )
            OutStr += "Newtype.";
        else
            OutStr += "Gundam.";
            
        break;
        
    case 13:
        OutStr += "Sexbot Mk-";
        
        random = g_Dice % 200 + 50;
        OutStr += toString( random );
        OutStr += ".";
        
        break;
        
    case 14:
        OutStr += "underage kids ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "(Who claim to be of age.)";
        else if( random <= 4 )
            OutStr += "(Who snuck in.)";
        else if( random <= 6 )
            OutStr += "(Who are somehow related to the Brothel Master, so its ok.)";
        else if( random <= 8 )
            OutStr += "(They paid, so who cares?)";
        else
            OutStr += "(We must corrupt them while they're still young.)";
            
        break;
        
    case 15:
        OutStr += "Grandpa Parkins from down the street.";
        break;
        
    case 16:
        OutStr += "the ... thing living ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "in the underwear drawer";
        else if( random <= 4 )
            OutStr += "in the sex-toy box";
        else if( random <= 6 )
            OutStr += "under the bed";
        else if( random <= 8 )
            OutStr += "in her shadow";
        else
            OutStr += "in her psyche";
            
        OutStr += ".";
        break;
        
    case 17:
        OutStr += "the senior member of the cult of ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "tentacles.";
        else if( random <= 4 )
            OutStr += "unending pain.";
        else if( random <= 6 )
            OutStr += "joy and happiness.";
        else if( random <= 8 )
            OutStr += "Whore Master developers.";
        else
            OutStr += "eunuchs.";
            
        break;
        
    case 18:
        OutStr += "this wierdo who appeared out of this blue box called a ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "TARDIS.";
        else
            OutStr += "TURDIS"; // How many people will say I made a spelling mistake?
            
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += " ";
            random = g_Dice % 10 + 1;
            
            if( random <= 2 )
                OutStr += "His female companion was in on the action too.";
            else if( random <= 4 )
                OutStr += "His mechanical dog was involved as well.";
            else if( random <= 6 )
                OutStr += "His female companion and mechanical dog did lewd things to each other and watched.";
        }
        
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion bdsm3
    
    OutStr += "\n";
    return OutStr;
}

std::string cGirls::GetRandomBeastString()
{
    int roll1 = 0, roll2 = 0, roll3 = 0, random = 0;
    /*char buffer[10];*/
    std::string OutStr;
    bool NeedAnd = false;
    
    OutStr += " was ";
    
    # pragma region beast1
    roll1 = g_Dice % 7 + 1;   // Remember to update this when new strings are added
    
    switch( roll1 )
    {
    case 1:
        OutStr += "filled with ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "vibrating";
        else if( random <= 4 )
            OutStr += "wiggling";
        else if( random <= 6 )
            OutStr += "living";
        else if( random <= 8 )
            OutStr += "energetic";
        else if( random <= 10 )
            OutStr += "big";
        else
            OutStr += "pokey";
            
        OutStr += " things that ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "tickled";
        else if( random <= 4 )
            OutStr += "pleasured";
        else if( random <= 6 )
            OutStr += "massaged";
        else
            OutStr += "scraped";
            
        OutStr += " her insides";
        break;
        
    case 2:
        OutStr += "forced against ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "a wall";
        else if( random <= 4 )
            OutStr += "a window";
        else if( random <= 6 )
            OutStr += "another client";
        else
            OutStr += "another girl";
            
        OutStr += " and told to ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "spread her legs";
        else if( random <= 4 )
            OutStr += "give up hope";
        else if( random <= 6 )
            OutStr += "hold on tight";
        else
            OutStr += "smile through it";
            
        break;
        
    case 3:
        OutStr += "worried by the ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "huge size";
        else if( random <= 4 )
            OutStr += "skill";
        else if( random <= 6 )
            OutStr += "reputation";
        else
            OutStr += "aggressiveness";
            
        OutStr += " of the client";
        break;
        
    case 4:
        OutStr += "stripped down to her ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "g-string";
        else if( random <= 4 )
            OutStr += "panties";
        else if( random <= 6 )
            OutStr += "bra and panties";
        else if( random <= 8 )
            OutStr += "teddy";
        else
            OutStr += "skin";
            
        OutStr += " and covered in ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "compliments";
        else if( random <= 4 )
            OutStr += "abuse";
        else if( random <= 6 )
            OutStr += "peanut butter";
        else if( random <= 8 )
            OutStr += "honey";
        else
            OutStr += "motor oil";
            
        break;
        
    case 5:
        OutStr += "chained up in the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "dungeon";
        else if( random <= 4 )
            OutStr += "den";
        else if( random <= 6 )
            OutStr += "kitchen";
        else if( random <= 8 )
            OutStr += "most public of places";
        else
            OutStr += "backyard";
            
        OutStr += " and her ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "arms";
        else if( random <= 4 )
            OutStr += "legs";
        else
            OutStr += "arms and legs";
            
        OutStr += " were lashed to posts";
        break;
        
    case 6:
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "tied up";
        else if( random <= 4 )
            OutStr += "wrapped up";
        else if( random <= 6 )
            OutStr += "trapped";
        else if( random <= 8 )
            OutStr += "bound";
        else
            OutStr += "covered";
            
        OutStr += " in ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "flesh";
        else if( random <= 4 )
            OutStr += "tentacles";
        else if( random <= 6 )
            OutStr += "cellophane";
        else if( random <= 8 )
            OutStr += "tape";
        else
            OutStr += "false promises";
            
        OutStr += " and ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "groped";
        else if( random <= 4 )
            OutStr += "tweaked";
        else if( random <= 6 )
            OutStr += "licked";
        else if( random <= 8 )
            OutStr += "spanked";
        else
            OutStr += "left alone";
            
        OutStr += " for hours";
        break;
        
    case 7:
        OutStr += "pushed to the limits of ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "flexibility";
        else if( random <= 4 )
            OutStr += "endurance";
        else if( random <= 6 )
            OutStr += "patience";
        else if( random <= 8 )
            OutStr += "consciousness";
        else
            OutStr += "sanity";
            
        OutStr += " and ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "cried out";
        else if( random <= 4 )
            OutStr += "swooned";
        else
            OutStr += "spasmed";
            
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion beast1
    
    // Part 2
    # pragma region beast2
    OutStr += ". ";
    
    roll2 = g_Dice % 9 + 1;
    
    switch( roll2 )
    {
    case 1:
        OutStr += "She ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "smoothly";
        else if( random <= 4 )
            OutStr += "roughly";
        else if( random <= 6 )
            OutStr += "lustily";
        else if( random <= 8 )
            OutStr += "repeatedly";
        else
            OutStr += "orgasmically";
            
        OutStr += " ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "fucked";
        else if( random <= 4 )
            OutStr += "railed";
        else if( random <= 6 )
            OutStr += "banged";
        else if( random <= 8 )
            OutStr += "screwed";
        else
            OutStr += "pleasured";
            
        break;
        
    case 2:
        OutStr += "She was ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "teased";
        else if( random <= 4 )
            OutStr += "taunted";
        else
            OutStr += "roughed up";
            
        OutStr += " and ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "orally";
        else if( random <= 4 )
            OutStr += "";         // MYR: This isn't a bug.  'physically violated' is redundant, so this just prints 'violated'
        else if( random <= 6 )
            OutStr += "mentally";
        else if( random <= 8 )
            OutStr += "repeatedly";
        else
            OutStr += "haughtily";
            
        OutStr += " violated by";
        break;
        
    case 3:
        OutStr += "She was drenched in ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "cum";
        else if( random <= 4 )
            OutStr += "sweat";
        else if( random <= 6 )
            OutStr += "broken hopes and dreams";
        else if( random <= 8 )
            OutStr += "Koolaid";
        else
            OutStr += "sticky secretions";
            
        OutStr += " by";
        break;
        
    case 4:
        OutStr += "She ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "moaned";
        else if( random <= 4 )
            OutStr += "winced";
        else if( random <= 6 )
            OutStr += "swooned";
        else if( random <= 8 )
            OutStr += "orgasmed";
        else
            OutStr += "begged for more";
            
        OutStr += " as her stomach repeatedly poked out from ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "thrusts";
        else if( random <= 4 )
            OutStr += "strokes";
        else if( random <= 6 )
            OutStr += "fistings";
        else
            OutStr += "a powerful fucking";
            
        OutStr += " by";
        break;
        
    case 5:
        OutStr += "She used her ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += "hands, ";
            NeedAnd = true;
        }
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += "feet, ";
            NeedAnd = true;
        }
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += "mouth, ";
            NeedAnd = true;
        }
        
        if( NeedAnd == true )
        {
            OutStr += "and ";
            NeedAnd = false;   // Just in case it's used again here
        }
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "pussy";
        else
            OutStr += "holes";
            
        OutStr += " to ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "please";
        else if( random <= 4 )
            OutStr += "pleasure";
        else
            OutStr += "tame";
            
        break;
        
    case 6:
        OutStr += "She shook with ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "orgasmic joy";
        else if( random <= 4 )
            OutStr += "searing pain";
        else if( random <= 6 )
            OutStr += "frustration";
        else if( random <= 8 )
            OutStr += "agony";
        else
            OutStr += "frustrated boredom";
            
        OutStr += " when fondled by";
        break;
        
    case 7:
        OutStr += "It felt like she was ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "ripping apart";
        else if( random <= 4 )
            OutStr += "exploding";
        else if( random <= 6 )
            OutStr += "imploding";
        else if( random <= 8 )
            OutStr += "nothing";
        else
            OutStr += "absent";
            
        OutStr += " when handled by";
        break;
        
    case 8:
        OutStr += "She passed out from ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "pleasure";
        else if( random <= 4 )
            OutStr += "pain";
        else if( random <= 6 )
            OutStr += "boredom";
        else if( random <= 8 )
            OutStr += "rough sex";
        else
            OutStr += "inactivity";
            
        OutStr += " from";
        break;
        
    case 9:
        OutStr += "She screamed as ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "wrenching pain";
        else if( random <= 4 )
            OutStr += "powerful orgasms";
        else if( random <= 6 )
            OutStr += "incredible sensations";
        else if( random <= 8 )
            OutStr += "freight trains";
        else
            OutStr += "lots and lots of nothing";
            
        OutStr += " thundered through her from";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion beast2
    
    // Part 3
    # pragma region beast3
    OutStr += " ";
    
    roll3 = g_Dice % 12 + 1;
    
    switch( roll3 )
    {
    case 1:
        OutStr += "the ravenous ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "tentacles.";
        else if( random <= 4 )
            OutStr += ", sex-starved essences of lust.";
        else if( random <= 6 )
            OutStr += "Balhannoth. (Monster Manual 4, pg. 15.)";
        else if( random <= 8 )
            OutStr += "priest.";
        else
            OutStr += "Yugoloth.";
            
        break;
        
    case 2:
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "an evil";
        else
            OutStr += "a misunderstood";
            
        OutStr += " ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "tengu.";
        else if( random <= 4 )
            OutStr += "Whore Master developer.";
        else if( random <= 6 )
            OutStr += "school girl.";
        else if( random <= 8 )
            OutStr += "garden hose.";
        else
            OutStr += "thought.";
            
        break;
        
    case 3:
        OutStr += "a major ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "demon";
        else if( random <= 4 )
            OutStr += "devil";
        else if( random <= 6 )
            OutStr += "oni";
        else if( random <= 8 )
            OutStr += "fire elemental";
        else if( random <= 10 )
            OutStr += "god";
        else
            OutStr += "Mr. Coffee";
            
        OutStr += " from the outer planes.";
        break;
        
    case 4:
        OutStr += "the angel.";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += " ('";
            random = g_Dice % 8 + 1;
            
            if( random <= 2 )
                OutStr += "You're very pretty";
            else if( random <= 4 )
                OutStr += "I was never here";
            else if( random <= 6 )
                OutStr += "I had a great time";
            else
                OutStr += "I didn't know my body could do that";
                
            OutStr += "' he said.)";
        }
        
        break;
        
    case 5:
        OutStr += "the ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "demon";
        else if( random <= 4 )
            OutStr += "major devil";
        else if( random <= 6 )
            OutStr += "oni";
        else if( random <= 8 )
            OutStr += "earth elemental";
        else if( random <= 10 )
            OutStr += "raging hormome beast";
        else
            OutStr += "Happy Fun Ball";
            
        OutStr += " with an urge to exercise his ";
        
        random = g_Dice % 30 + 20;
        OutStr += toString( random );
        OutStr += " cocks and ";
        
        random = g_Dice % 30 + 20;
        OutStr += toString( random );
        OutStr += " claws.";
        
        break;
        
    case 6:
        OutStr += "the swamp thing with (wait for it) swamp tentacles!";
        break;
        
    case 7:
        OutStr += "the pirnja gestalt. (The revolution is coming.)";
        break;
        
    case 8:
        OutStr += "the color ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "purple";
        else if( random <= 4 )
            OutStr += "seven";  // MYR: Not a mistake. I meant to write 'seven'.
        else if( random <= 6 )
            OutStr += "mauve";
        else if( random <= 8 )
            OutStr += "silver";
        else if( random <= 10 )
            OutStr += "ochre";
        else
            OutStr += "pale yellow";
            
        OutStr += ".";
        break;
        
    case 9:
        random = g_Dice % 10 + 5;
        OutStr += toString( random );
        
        OutStr += " werewolves wearing ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "true";
        else if( random <= 4 )
            OutStr += "minor artifact";
        else if( random <= 6 )
            OutStr += "greater artifact";
        else if( random <= 10 )
            OutStr += "godly";
        else
            OutStr += "near omnipitent";
            
        OutStr += " rings of the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "eternal";
        else if( random <= 4 )
            OutStr += "body breaking";
        else if( random <= 6 )
            OutStr += "vorporal";
        else if( random <= 10 )
            OutStr += "transcendent";
        else
            OutStr += "incorporeal";
            
        OutStr += " hard-on.";
        break;
        
    case 10:
        random = g_Dice % 10 + 5;
        OutStr += toString( random );
        
        OutStr += " Elder Gods.";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 4 )
        {
            OutStr += " (She thought ";
            
            random = g_Dice % 12 + 1;
            
            if( random <= 2 )
                OutStr += "Cthulhu";
            else if( random <= 4 )
                OutStr += "Hastur";
            else if( random <= 6 )
                OutStr += "an Old One";
            else if( random <= 8 )
                OutStr += "Shub-Niggurath";
            else if( random <= 10 )
                OutStr += "Nyarlathotep";
            else
                OutStr += "Yog-Sothoth";
                
            OutStr += " was amongst them, but blacked out after a minute or so.)";
        }
        
        break;
        
    case 11:
        OutStr += "the level ";
        
        random = g_Dice % 20 + 25;
        OutStr += toString( random );
        
        OutStr += " epic paragon ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "troll";
        else if( random <= 4 )
            OutStr += "beholder";
        else if( random <= 6 )
            OutStr += "displacer beast";
        else if( random <= 8 )
            OutStr += "ettin";
        else if( random <= 10 )
            OutStr += "gargoyle";
        else
            OutStr += "fire extinguisher";
            
        OutStr += " with ";
        
        random = g_Dice % 20 + 20;
        OutStr += toString( random );
        OutStr += " strength and ";
        
        random = g_Dice % 20 + 20;
        OutStr += toString( random );
        OutStr += " constitution.";
        
        break;
        
    case 12:
        OutStr += "the phalanx of ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "horny orcs.";
        else if( random <= 4 )
            OutStr += "goblins.";
        else if( random <= 6 )
            OutStr += "sentient marbles.";
        else if( random <= 8 )
            OutStr += "living garden gnomes.";
        else
            OutStr += "bugbears.";
            
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion beast3
    
    OutStr += "\n";
    return OutStr;
}

std::string cGirls::GetRandomLesString()
{
    int roll1 = 0, roll2 = 0, roll3 = 0, random = 0, plus = 0;
    std::string OutStr;
    /*char buffer[10];*/
    
    OutStr += " ";
    
    // Part1
    # pragma region les1
    roll1 = g_Dice % 6 + 1;   // Remember to update this when new strings are added
    
    switch( roll1 )
    {
    case 1:
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "aggressively";
        else if( random <= 4 )
            OutStr += "tenderly";
        else if( random <= 6 )
            OutStr += "slowly";
        else if( random <= 8 )
            OutStr += "authoratively";
        else
            OutStr += "violently";
            
        OutStr += " ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "straddled";
        else if( random <= 4 )
            OutStr += "scissored";
        else if( random <= 6 )
            OutStr += "symmetrically docked with";
        else if( random <= 8 )
            OutStr += "cowgirled";
        else
            OutStr += "69ed";
            
        OutStr += " the woman";
        break;
        
    case 2:
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "shaved her";
        else
            OutStr += "was shaved";
            
        OutStr += " with a +";
        
        plus = g_Dice % 7 + 4;
        OutStr += toString( plus );
        OutStr += " ";
        
        random = g_Dice % 14 + 1;
        
        if( random <= 2 )
            OutStr += "vorporal broadsword";
        else if( random <= 4 )
        {
            OutStr += "dagger, +";
            plus = plus + g_Dice % 5 + 2;
            OutStr += toString( plus );
            OutStr += " vs pubic hair";
        }
        else if( random <= 6 )
            OutStr += "flaming sickle";
        else if( random <= 8 )
            OutStr += "lightning burst bo-staff";
        else if( random <= 10 )
            OutStr += "human bane greatsword";
        else if( random <= 12 )
            OutStr += "acid burst warhammer";
        else
            OutStr += "feral halfling";
            
        break;
        
    case 3:
        OutStr += "had a ";
        
        random = g_Dice % 14 + 1;
        
        if( random <= 2 )
            OutStr += "pleasant";
        else if( random <= 4 )
            OutStr += "long";
        else if( random <= 6 )
            OutStr += "heartfelt";
        else if( random <= 8 )
            OutStr += "deeply personal";
        else if( random <= 10 )
            OutStr += "emotional";
        else if( random <= 12 )
            OutStr += "angry";
        else
            OutStr += "violent";
            
        OutStr += " conversation with her lady-client about ";
        
        random = g_Dice % 16 + 1;
        
        if( random <= 2 )
            OutStr += "sadism";
        else if( random <= 4 )
            OutStr += "particle physics";
        else if( random <= 6 )
            OutStr += "domination";
        else if( random <= 8 )
            OutStr += "submission";
        else if( random <= 10 )
            OutStr += "brewing poisons";
        else if( random <= 12 )
            OutStr += "flower arranging";
        else if( random <= 14 )
            OutStr += "the Brothel Master";
        else
            OutStr += "assassination techniques";
            
        break;
        
    case 4:
        OutStr += "massaged the woman with ";
        
        // MYR: Ok, I know I'm being super-silly
        random = g_Dice % 20 + 1;
        
        if( random <= 2 )
            OutStr += "bath oil";
        else if( random <= 4 )
            OutStr += "aloe vera";
        else if( random <= 6 )
            OutStr += "the tears of Chuck Norris's many victims";
        else if( random <= 8 )
            OutStr += "the blood of innocent angels";
        else if( random <= 10 )
            OutStr += "Unicorn blood";
        else if( random <= 12 )
            OutStr += "Unicorn's tears";
        else if( random <= 14 )
            OutStr += "a strong aphrodisiac";
        else if( random <= 16 )
            OutStr += "oil of greater breast growth";
        else if( random <= 18 )
            OutStr += "potent oil of massive breast growth";
        else
            OutStr += "oil of camel-toe growth";
            
        break;
        
    case 5:
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "put a ball gag and blindfolded on";
        else
            OutStr += "put a sensory deprivation hood on";
            
        OutStr += ", was ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "lashed";
        else if( random <= 4 )
            OutStr += "cuffed";
        else if( random <= 6 )
            OutStr += "tied";
        else
            OutStr += "chained";
            
        OutStr += " to a ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "bed";
        else if( random <= 4 )
            OutStr += "bench";
        else if( random <= 6 )
            OutStr += "table";
        else
            OutStr += "post";
            
        OutStr += " and ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "symmetrically docked";
        else if( random <= 4 )
            OutStr += "69ed";
        else if( random <= 6 )
            OutStr += "straddled";
        else
            OutStr += "scissored";
            
        break;
        
    case 6:
        // MYR: This is like a friggin movie! The epic story of the whore and her customer.
        OutStr += "looked at the woman across from her. ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "Position";
        else if( random <= 4 )
            OutStr += "Toy";
        else if( random <= 6 )
            OutStr += "Oil";
        else if( random <= 8 )
            OutStr += "Bed sheet color";
        else
            OutStr += "Price";
            
        OutStr += " was to be ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "a trial";
        else if( random <= 4 )
            OutStr += "decided";
        else if( random <= 6 )
            OutStr += "resolved";
        else
            OutStr += "dictated";
            
        OutStr += " by combat. Both had changed into ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "string bikinis";
        else if( random <= 4 )
            OutStr += "lingerie";
        else if( random <= 6 )
            OutStr += "body stockings";
        else if( random <= 8 )
            OutStr += "their old school uniforms";
        else
            OutStr += "dominatrix outfits";
            
        OutStr += " and wielded ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "whips";
        else if( random <= 4 )
            OutStr += "staves";
        else if( random <= 6 )
            OutStr += "boxing gloves";
        else
            OutStr += "cat-o-nine tails";
            
        OutStr += " of ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "explosive orgasms";
        else if( random <= 4 )
            OutStr += "clothes shredding";
        else if( random <= 6 )
            OutStr += "humiliation";
        else if( random <= 8 )
            OutStr += "subjugation";
        else
            OutStr += "brutal stunning";
            
        OutStr += ". ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            OutStr += "They stared at each other across the ";
            random = g_Dice % 8 + 1;
            
            if( random <= 2 )
                OutStr += "mud";
            else if( random <= 4 )
                OutStr += "jello";
            else if( random <= 6 )
                OutStr += "whip cream";
            else
                OutStr += "clothes-eating slime";
                
            OutStr += " pit.";
        }
        
        OutStr += " A bell sounded! They charged and ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "dueled";
        else if( random <= 4 )
            OutStr += "fought it out";
        else if( random <= 6 )
            OutStr += "battled";
            
        OutStr += "!\n";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 6 ) // MYR: small bias for the customer
            OutStr += "The customer won";
        else
            OutStr += "The customer was vanquished";
            
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion les1
    
    OutStr += ". ";
    
    // Part 2
    # pragma region les2
    roll2 = g_Dice % 8 + 1;
    
    switch( roll2 )
    {
    case 1:
        OutStr += "She was ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "tormented";
        else if( random <= 4 )
            OutStr += "teased";
        else if( random <= 6 )
            OutStr += "massaged";
        else if( random <= 8 )
            OutStr += "frustrated";
        else
            OutStr += "satisfied";
            
        OutStr += " with ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "great care";
        else if( random <= 4 )
            OutStr += "deva feathers";
        else if( random <= 6 )
            OutStr += "drug-soaked sex toys";
        else if( random <= 8 )
            OutStr += "extreme skill";
        else
            OutStr += "wild abandon";
            
        OutStr += " by";
        break;
        
    case 2:
        // Case 1 reversed and reworded
        OutStr += "She used ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "phoenix down";
        else if( random <= 4 )
            OutStr += "deva feathers";
        else if( random <= 6 )
            OutStr += "drug-soaked sex toys";
        else if( random <= 8 )
            OutStr += "restraints";
        else
            OutStr += "her wiles";
            
        OutStr += " to ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "torment";
        else if( random <= 4 )
            OutStr += "tease";
        else if( random <= 6 )
            OutStr += "massage";
        else if( random <= 8 )
            OutStr += "frustrate";
        else
            OutStr += "satisfy";
            
        break;
        
    case 3:
        OutStr += "She ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "fingered";
        else if( random <= 4 )
            OutStr += "teased";
        else if( random <= 6 )
            OutStr += "caressed";
        else if( random <= 8 )
            OutStr += "fondled";
        else
            OutStr += "pinched";
            
        OutStr += " the client's ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "clit";
        else if( random <= 4 )
            OutStr += "clitorus";
        else
            OutStr += "love bud";
            
        OutStr += " and expertly elicited orgasm after orgasm from";
        break;
        
    case 4:
        OutStr += "Her ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "clit";
        else if( random <= 4 )
            OutStr += "clitorus";
        else
            OutStr += "love bud";
            
        OutStr += " was ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "fingered";
        else if( random <= 4 )
            OutStr += "teased";
        else if( random <= 6 )
            OutStr += "caressed";
        else if( random <= 8 )
            OutStr += "fondled";
        else
            OutStr += "pinched";
            
        OutStr += " and she orgasmed repeatedly under the expert touch of";
        break;
        
    case 5:
        OutStr += "She ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "clam wrestled";
        else if( random <= 4 )
            OutStr += "rubbed";
        else if( random <= 6 )
            OutStr += "attacked";
        else
            OutStr += "hammered";
            
        OutStr += " the client's ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "clit";
        else if( random <= 4 )
            OutStr += "clitorus";
        else if( random <= 6 )
            OutStr += "love bud";
        else
            OutStr += "vagina";
            
        OutStr += " causing waves of orgasms to thunder through";
        break;
        
    case 6:
        OutStr += "She ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "single mindedly";
        else if( random <= 4 )
            OutStr += "repeatedly";
        else
            OutStr += "roughly";
            
        OutStr += " ";
        
        random = g_Dice % 2 + 1;
        
        if( random <= 2 )
            OutStr += "rubbed";
        else if( random <= 4 )
            OutStr += "fondled";
        else if( random <= 6 )
            OutStr += "prodded";
        else if( random <= 8 )
            OutStr += "attacked";
        else if( random <= 10 )
            OutStr += "tongued";
        else
            OutStr += "licked";
            
        OutStr += " the client's g-spot. Wave after wave of ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "orgasms";
        else if( random <= 4 )
            OutStr += "pleasure";
        else if( random <= 6 )
            OutStr += "powerful sensations";
        else
            OutStr += "indescribable joy";
            
        OutStr += " ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "rushed";
        else if( random <= 4 )
            OutStr += "thundered";
        else if( random <= 6 )
            OutStr += "cracked";
        else
            OutStr += "pounded";
            
        OutStr += " through";
        break;
        
    case 7:
        OutStr += "Wave after wave of ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "orgasms";
        else if( random <= 4 )
            OutStr += "back-stretching joy";
        else if( random <= 6 )
            OutStr += "madness";
        else
            OutStr += "incredible feeling";
            
        OutStr += " ";
        
        random = g_Dice % 8 + 1;
        
        if( random <= 2 )
            OutStr += "throbbed";
        else if( random <= 4 )
            OutStr += "shook";
        else if( random <= 6 )
            OutStr += "arced";
        else
            OutStr += "stabbed";
            
        OutStr += " through her as she was ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "single mindedly";
        else if( random <= 4 )
            OutStr += "repeatedly";
        else
            OutStr += "roughly";
            
        OutStr += " ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "rubbed";
        else if( random <= 4 )
            OutStr += "fondled";
        else if( random <= 6 )
            OutStr += "prodded";
        else if( random <= 8 )
            OutStr += "attacked";
        else if( random <= 10 )
            OutStr += "tongued";
        else
            OutStr += "licked";
            
        OutStr += " by";
        break;
        
    case 8:
        // MYR: I just remembered about \n
        OutStr += "Work stopped ";
        
        random = g_Dice % 14 + 1;
        
        if( random <= 2 )
            OutStr += "in the brothel";
        else if( random <= 4 )
            OutStr += "on the street";
        else if( random <= 6 )
            OutStr += "all over the block";
        else if( random <= 8 )
            OutStr += "in the town";
        else if( random <= 10 )
            OutStr += "within the country";
        else  if( random <= 12 )
            OutStr += "over the whole planet";
        else  if( random <= 14 )
            OutStr += "within the solar system";
        else
            OutStr += "all over the galactic sector";
            
            
        OutStr += ". Everything was drowned out by:\n\n";
        OutStr += "Ahhhhh!\n\n";
        
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
        {
            random = g_Dice % 6 + 1;
            
            if( random <= 2 )
                OutStr += "For the love... of aaaaahhhhh mercy.  No nnnnnnnnh more!\n\n";
            else if( random <= 4 )
                OutStr += "oooooOOOOOO YES! ahhhhhhHHHH!\n\n";
            else
                OutStr += "nnnnnhhh nnnnnhhhh NNNHHHHHH!!!!\n\n";
        }
        
        OutStr += "Annnnnhhhhhaaa!\n\n";
        OutStr += "AHHHHHHHH! I'm going to ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "CCCUUUUUUMMMMMMMM!!!!!";
        else if( random <= 4 )
            OutStr += "EEEXXXXXPLLLOOODDDDEEEE!!!";
        else if( random <= 6 )
            OutStr += "DIEEEEEE!";
        else if( random <= 8 )
            OutStr += "AHHHHHHHHHHH!!!!";
        else if( random <= 10 )
            OutStr += "BRRRREEEEAAAAKKKKKK!!!!";
        else
            OutStr += "WAAAAHHHHHOOOOOOO!!!";
            
        OutStr += "\nfrom ";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion les2
    
    OutStr += " ";
    
    // Part 3
    # pragma region les3
    
    // For case 2
    int BrothelNo = -1, NumGirlsInBroth = -1;
    Girl* TempGPtr = nullptr;
    
    roll3 = g_Dice % 6 + 1;
    
    switch( roll3 )
    {
    case 1:
        OutStr += "the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "prostitute";
        else if( random <= 4 )
            OutStr += "street worker";
        else if( random <= 6 )
            OutStr += "hooker";
        else if( random <= 8 )
            OutStr += "street walker";
        else
            OutStr += "working girl";
            
        OutStr += " from a ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "friendly";
        else if( random <= 4 )
            OutStr += "rival";
        else if( random <= 6 )
            OutStr += "hostile";
        else
            OutStr += "feeder";
            
        OutStr += " brothel.";
        break;
        
    case 2:
        BrothelNo = g_Dice % g_Brothels.GetNumBrothels();
        NumGirlsInBroth = g_Brothels.GetNumGirls( BrothelNo );
        random = g_Dice % NumGirlsInBroth;
        TempGPtr = g_Brothels.GetGirl( BrothelNo, random );
        
        OutStr += TempGPtr->m_Realname;
        OutStr += " from ";
        OutStr += g_Brothels.GetName( BrothelNo );
        OutStr += " brothel.";
        
        BrothelNo = -1;        // MYR: Paranoia
        NumGirlsInBroth = -1;
        TempGPtr = nullptr;
        break;
        
    case 3:
        OutStr += "the ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "sexy";
        else if( random <= 4 )
            OutStr += "rock hard";
        else if( random <= 6 )
            OutStr += "hot";
        else if( random <= 8 )
            OutStr += "androgonous";
        else if( random <= 10 )
            OutStr += "spirited";
        else
            OutStr += "exuberant";
            
        OutStr += " MILF.";
        break;
        
    case 4:
        OutStr += "the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 5 )
            OutStr += "senior";
        else
            OutStr += "junior";
            
        OutStr += " ";
        
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "Sorceress";
        else if( random <= 4 )
            OutStr += "Warrioress";
        else if( random <= 6 )
            OutStr += "Priestess";
        else if( random <= 8 )
            OutStr += "Huntress";
        else if( random <= 10 )
            OutStr += "Amazon";
        else
            OutStr += "Druidess";
            
        OutStr += " of the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "Hidden";
        else if( random <= 4 )
            OutStr += "Silent";
        else if( random <= 6 )
            OutStr += "Masters";
        else if( random <= 8 )
            OutStr += "Scarlet";
        else
            OutStr += "Resolute";
            
            
        OutStr += " ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "Hand";
        else if( random <= 4 )
            OutStr += "Dagger";
        else if( random <= 6 )
            OutStr += "Will";
        else if( random <= 8 )
            OutStr += "League";
        else
            OutStr += "Hearts";
            
        OutStr += ".";
        break;
        
    case 5:
        OutStr += "the ";
        
        random = g_Dice % 6 + 1;
        
        if( random <= 2 )
            OutStr += "high-ranking";
        else if( random <= 4 )
            OutStr += "mid-tier";
        else
            OutStr += "low-ranking";
            
        OutStr += " ";
        
        random = g_Dice % 14 + 1;
        
        if( random <= 2 )
            OutStr += "elf";
        else if( random <= 4 )
            OutStr += "woman";     // MYR: Human assumed
        else if( random <= 6 )
            OutStr += "dryad";
        else if( random <= 8 )
            OutStr += "succubus";
        else if( random <= 10 )
            OutStr += "nymph";
        else if( random <= 12 )
            OutStr += "eyrine";
        else
            OutStr += "cat girl";
            
        OutStr += " from the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "Nymphomania";
        else if( random <= 4 )
            OutStr += "Satyriasis";
        else if( random <= 6 )
            OutStr += "Women Who Love Sex";
        else if( random <= 8 )
            OutStr += "Real Women Don't Marry";
        else
            OutStr += "Monster Sex is Best";
            
        OutStr += " ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "support group";
        else if( random <= 4 )
            OutStr += "league";
        else if( random <= 6 )
            OutStr += "club";
        else if( random <= 8 )
            OutStr += "faction";
        else
            OutStr += "guild";
            
        OutStr += ".";
        break;
        
    case 6:
        OutStr += "the ";
        
        random = g_Dice % 10 + 1;
        
        if( random <= 2 )
            OutStr += "disguised";
        else if( random <= 4 )
            OutStr += "hot";
        else if( random <= 6 )
            OutStr += "sexy";
        else if( random <= 8 )
            OutStr += "curvacious";
        else
            OutStr += "sultry";
            
        OutStr += " ";
        
        // MYR: Covering the big fetishes/stereotpes
        random = g_Dice % 12 + 1;
        
        if( random <= 2 )
            OutStr += "idol singer";
        else if( random <= 4 )
            OutStr += "princess";
        else if( random <= 6 )
            OutStr += "school girl";
        else if( random <= 8 )
            OutStr += "nurse";
        else if( random <= 10 )
            OutStr += "maid";
        else
            OutStr += "waitress";
            
        OutStr += ".";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    # pragma endregion les3
    OutStr += "\n";
    return OutStr;
}

// MYR: Burned out before anal. Someone else feeling creative?

std::string cGirls::GetRandomAnalString()
{
    int roll1 = 0, roll2 = 0, roll3 = 0;
    std::string OutStr;
    
    OutStr += " ";
    
    // Part 1
    #pragma region anal1
    roll1 = g_Dice % 10 + 1;   // Remember to update this when new strings are added
    
    switch( roll1 )
    {
    case 1:
        OutStr += "";
        break;
        
    case 2:
        OutStr += "";
        break;
        
    case 3:
        OutStr += "";
        break;
        
    case 4:
        OutStr += "";
        break;
        
    case 5:
        OutStr += "";
        break;
        
    case 6:
        OutStr += "";
        break;
        
    case 7:
        OutStr += "";
        break;
        
    case 8:
        OutStr += "";
        break;
        
    case 9:
        OutStr += "";
        break;
        
    case 10:
        OutStr += "";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    #pragma endregion anal1
    
    OutStr += ". ";
    
    // Part 2
    #pragma region anal2
    roll2 = g_Dice % 10 + 1;
    
    switch( roll2 )
    {
    case 1:
        OutStr += "";
        break;
        
    case 2:
        OutStr += "";
        break;
        
    case 3:
        OutStr += "";
        break;
        
    case 4:
        OutStr += "";
        break;
        
    case 5:
        OutStr += "";
        break;
        
    case 6:
        OutStr += "";
        break;
        
    case 7:
        OutStr += "";
        break;
        
    case 8:
        OutStr += "";
        break;
        
    case 9:
        OutStr += "";
        break;
        
    case 10:
        OutStr += "";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    #pragma endregion anal2
    
    OutStr += " ";
    
    // Part 3
    #pragma endregion anal3
    roll3 = g_Dice % 10 + 1;
    
    switch( roll3 )
    {
    case 1:
        OutStr += "";
        break;
        
    case 2:
        OutStr += "";
        break;
        
    case 3:
        OutStr += "";
        break;
        
    case 4:
        OutStr += "";
        break;
        
    case 5:
        OutStr += "";
        break;
        
    case 6:
        OutStr += "";
        break;
        
    case 7:
        OutStr += "";
        break;
        
    case 8:
        OutStr += "";
        break;
        
    case 9:
        OutStr += "";
        break;
        
    case 10:
        OutStr += "";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    #pragma endregion anal3
    
    return OutStr;
}

// ----- Combat

Uint8 cGirls::girl_fights_girl( Girl* a, Girl* b )
{
    // NB: For girls exploring the catacombs, a is the character, b the monster
    // Return value of 1 means a (the girl) won
    //   "      "    " 2  "   b (the monster) won
    //   "      "    " 0  "   it was a draw
    
    // MYR: Sanity checks on incorporial. It is actually possible (but very rare)
    //      for both girls to be incorporial.
    if( a->has_trait( "Incorporial" ) && b->has_trait( "Incorporial" ) )
    {
        g_LogFile.ss()  << "\ngirl_fights_girl: Both " << a->m_Realname << " and " << b->m_Realname
                << " are incorporial, so the fight is a draw.\n";
        return 0;
    }
    else if( a->has_trait( "Incorporial" ) )
    {
        g_LogFile.ss()  << "\ngirl_fights_girl: " << a->m_Realname << " is incorporial, so she wins.\n";
        return 1;
    }
    else if( a->has_trait( "Incorporial" ) )
    {
        g_LogFile.ss()  << "\ngirl_fights_girl: " << b->m_Realname << " is incorporial, so she wins.\n";
        return 2;
    }
    
    int a_dodge = 0;
    int b_dodge = 0;
    u_int a_attack = SKILL_COMBAT;  // determined later, defaults to combat
    u_int b_attack = SKILL_COMBAT;
    
    if( a == nullptr || b == nullptr )
        return 0;
        
    // first determine what skills they will fight with
    // girl a
    if( g_Girls.GetSkill( a, SKILL_COMBAT ) >= g_Girls.GetSkill( a, SKILL_MAGIC ) )
        a_attack = SKILL_COMBAT;
    else
        a_attack = SKILL_MAGIC;
        
    // girl b
    if( g_Girls.GetSkill( b, SKILL_COMBAT ) >= g_Girls.GetSkill( b, SKILL_MAGIC ) )
        b_attack = SKILL_COMBAT;
    else
        b_attack = SKILL_MAGIC;
        
    // determine dodge ability
    // girl a
    if( ( g_Girls.GetStat( a, STAT_AGILITY ) - g_Girls.GetStat( a, STAT_TIREDNESS ) ) < 0 )
        a_dodge = 0;
    else
        a_dodge = ( g_Girls.GetStat( a, STAT_AGILITY ) - g_Girls.GetStat( a, STAT_TIREDNESS ) );
        
    // girl b
    if( ( g_Girls.GetStat( b, STAT_AGILITY ) - g_Girls.GetStat( b, STAT_TIREDNESS ) ) < 0 )
        b_dodge = 0;
    else
        b_dodge = ( g_Girls.GetStat( b, STAT_AGILITY ) - g_Girls.GetStat( b, STAT_TIREDNESS ) );
        
    g_LogFile.ss() << "Girl vs. Girl: " << a->m_Realname << " fights " << b->m_Realname << "\n";
    g_LogFile.ss() << "\t" << a->m_Realname << ": Health " << a->health() << ", Dodge " << a_dodge << ", Mana " << a->mana() << "\n";
    g_LogFile.ss() << "\t" << b->m_Realname << ": Health " << b->health() << ", Dodge " << b_dodge << ", Mana " << b->mana() << "\n";
    g_LogFile.ssend();
    
    Girl* Attacker = b;
    Girl* Defender = a;
    unsigned int attack = 0;
    int dodge = a_dodge;
    int attack_count = 0;
    
    /*int winner = 0; // 1 for a, 2 for b*/
    while( 1 )
    {
        if( g_Girls.GetStat( a, STAT_HEALTH ) <= 20 )
        {
            g_Girls.UpdateEnjoyment( a, ACTION_COMBAT, -1, true );
            /*winner = 2; set not used */
            break;
        }
        else if( g_Girls.GetStat( b, STAT_HEALTH ) <= 20 )
        {
            g_Girls.UpdateEnjoyment( b, ACTION_COMBAT, -1, true );
            /*winner = 1; set not used */
            break;
        }
        
        if( attack_count > 1000 ) // if the fight's not over after 1000 blows, call it a tie
        {
            g_LogFile.ss()  << "The fight has gone on for too long, over 1000 (attempted) blows either way. Calling it a draw.";
            g_LogFile.ssend();
            
            return 0;
        }
        
        attack_count++;
        
        if( Attacker == a )
        {
            Attacker = b;
            attack = b_attack;
            Defender = a;
            b_dodge = dodge;
            dodge = a_dodge;
        }
        else if( Attacker == b )
        {
            Attacker = a;
            attack = a_attack;
            Defender = b;
            a_dodge = dodge;
            dodge = b_dodge;
        }
        
        // Girl attacks
        g_LogFile.ss() << "\t\t" << Attacker->m_Realname << " attacks: ";
        
        if( attack == SKILL_MAGIC )
        {
            if( Attacker->mana() < 7 )
                g_LogFile.ss()  << "Insufficient mana: using combat";
            else
            {
                Attacker->mana( -7 );
                g_LogFile.ss()  << "Casts a spell (mana now " << Attacker->mana() << ")";
            }
        }
        else
            g_LogFile.ss()  << "Using physical attack";
            
        g_LogFile.ssend();
        
        int girl_attack_chance = g_Girls.GetSkill( Attacker, attack );
        int die_roll = g_Dice.d100();
        
        g_LogFile.ss()  << "\t\t" << "Attack chance: " << girl_attack_chance << " Die roll: " << die_roll;
        g_LogFile.ssend();
        
        
        if( die_roll > girl_attack_chance )
            g_LogFile.ss()  << "\t\t\t" << "Miss!";
        else
        {
            int damage = 5;
            g_LogFile.ss()  << "\t\t\t" << "Hit! base damage is " << damage << ". ";
            
            /*
            *       she has hit now calculate how much damage will be done
            *
            *       magic works better than conventional violence.
            *       Link this amount of mana remaining?
            */
            int bonus = 0;
            
            if( attack == SKILL_MAGIC )
                bonus = g_Girls.GetSkill( Attacker, SKILL_MAGIC ) / 5 + 2;
            else
                bonus = g_Girls.GetSkill( Attacker, SKILL_COMBAT ) / 10;
                
            g_LogFile.ss()  << "Bonus damage is " << bonus << ".";
            g_LogFile.ssend();
            damage += bonus;
            
            die_roll = g_Dice.d100();
            
            // Defender attempts Dodge
            g_LogFile.ss()  << "\t\t\t\t" << Defender->m_Realname << " tries to dodge: needs " << dodge << ", gets " << die_roll << ": ";
            
            if( die_roll <= dodge )
            {
                g_LogFile.ss()  << "Success!";
                g_LogFile.ssend();
            }
            else
            {
                g_LogFile.ss()  << "Failure! ";
                g_LogFile.ssend();
                
                //int con_mod = Defender->m_Stats[STAT_CONSTITUTION] / 10;
                int con_mod = g_Girls.GetStat( Defender, STAT_CONSTITUTION ) / 10;
                int ActualDmg = damage - con_mod;
                
                if( ActualDmg <= 0 )
                    g_LogFile.ss() << "\t\t\t\t" << Defender->m_Realname << " shrugs it off.";
                else
                {
                    g_Girls.UpdateStat( Defender, STAT_HEALTH, -ActualDmg );
                    g_LogFile.ss() << "\t\t\t\t" << Defender->m_Realname << " takes " << damage  << " damage, less " << con_mod << " for CON\n";
                    g_LogFile.ss() << "\t\t\t\t\t" << "New health value = " << Defender->health();
                }
            } // Didn't dodge
        }     // Hit
        
        g_LogFile.ssend();
        
        // update girls dodge ability
        if( ( dodge - 1 ) < 0 )
            dodge = 0;
        else
            dodge--;
            
        // Check if girl 2 (the Defender) has been defeated
        // Highest damage is 27 pts.  Checking for health between 20 and 30 is a bug as it can be stepped over.
        if( g_Girls.GetStat( Defender, STAT_HEALTH ) <= 40 && g_Girls.GetStat( Defender, STAT_HEALTH ) >= 10 )
        {
            g_Girls.UpdateEnjoyment( Defender, ACTION_COMBAT, -1, true );
            break;
        }  // if defeated
    }  // while (1)
    
    // Girls exploring catacombs: Girl is "a" - and thus wins
    if( Attacker == a )
    {
        g_LogFile.ss() << a->m_Realname << " WINS!";
        g_LogFile.ssend();
        
        g_Girls.UpdateEnjoyment( a, ACTION_COMBAT, +1, true );
        
        return 1;
    }
    
    if( Attacker == b ) // Catacombs: Monster wins
    {
        g_LogFile.ss()  << b->m_Realname << " WINS!";
        g_LogFile.ssend();
        
        g_Girls.UpdateEnjoyment( b, ACTION_COMBAT, +1, true );
        
        return 2;
    }
    
    g_LogFile.ss()  << "ERROR: cGirls::girl_fights_girl - Shouldn't reach the function bottom.";
    g_LogFile.ssend();
    
    return 0;
}

bool cGirls::GirlInjured( Girl* girl, unsigned int unModifier )
{
    // modifier: 5 = 5% chance, 10 = 10% chance
    /*
     *  WD  Injury was only possible if girl is pregnant or
     *      hasn't got the required traits.
     *
     *      Now check for injury first
     *      Use usigned int so can't pass negative chance
     */
//  bool injured = false;
    std::string message;
    int nMod    = static_cast<int>( unModifier );
    
    // Sanity check, Can't get injured
    if( girl->has_trait( "Incorporial" ) )
        return false;
        
    if( girl->has_trait( "Fragile" ) )
        nMod += nMod;   // nMod *= 2;
        
    if( girl->has_trait( "Tough" ) )
        nMod /= 2;
        
    // Did the girl get injured
    if( !g_Dice.percent( nMod ) )
        return false;
        
    /*
     *  INJURY PROCESSING
     *  Only injured girls continue past here
     */
    
    // getting hurt badly could lead to scars
    if(
        g_Dice.percent( nMod * 2 ) &&
        !girl->has_trait( "Small Scars" ) &&
        !girl->has_trait( "Cool Scars" ) &&
        !girl->has_trait( "Horrific Scars" )
    )
    {
        //injured = true;
        int chance = g_Dice % 6;
        
        if( chance == 0 )
        {
            girl->add_trait( "Horrific Scars", false );
            message = "She was badly injured, and now has to deal with Horrific Scars.";
        }
        else if( chance <= 2 )
        {
            girl->add_trait( "Small Scars", false );
            message = "She was injured and now has a couple of Small Scars.";
        }
        else
        {
            girl->add_trait( "Cool Scars", false );
            message = "She was injured and scarred. As scars go however, at least they are pretty Cool Scars.";
        }
        
        girl->m_Events.AddMessage( message, IMGTYPE_PROFILE, EVENT_WARNING );
    }
    
    // in rare cases, she might even lose an eye
    if(
        g_Dice.percent( ( nMod / 2 ) ) &&
        !girl->has_trait( "One Eye" ) &&
        !girl->has_trait( "Eye Patch" )
    )
    {
        //injured = true;
        int chance = g_Dice % 3;
        
        if( chance == 0 )
        {
            girl->add_trait( "One Eye", false );
            message = "Oh, no! She was badly injured, and now only has One Eye!";
        }
        else
        {
            girl->add_trait( "Eye Patch", false );
            message = "She was injured and lost an eye, but at least she has a cool Eye Patch to wear.";
        }
        
        girl->m_Events.AddMessage( message, IMGTYPE_PROFILE, EVENT_WARNING );
    }
    
    // or become fragile
    if(
        g_Dice.percent( ( nMod / 2 ) )
        && !girl->has_trait( "Fragile" )
    )
    {
        //injured = true;
        girl->add_trait( "Fragile", false );
        message = "Her body has become rather Fragile due to the extent of her injuries.";
        girl->m_Events.AddMessage( message, IMGTYPE_PROFILE, EVENT_WARNING );
    }
    
    // and if pregnant, she might lose the baby; I'll assume inseminations can't be aborted so easily
    if( girl->carrying_human() && g_Dice.percent( ( nMod * 2 ) ) )
    {
        // unintended abortion time
        //injured = true;
        girl->clear_pregnancy();
        girl->happiness( -20 );
        girl->spirit( -5 );
        message = "Her unborn child has been lost due to the injuries she sustained, leaving her quite distraught.";
        girl->m_Events.AddMessage( message, IMGTYPE_PROFILE, EVENT_WARNING );
    }
    
    // Lose between 5 - 14 hp
    girl->health( -5 - g_Dice % 10 );
    
    return true;
}

// MYR: Centralized the damage calculation and added in traits that should apply
//      This fn and TakeCombatDamage(..) are in the attack/defense/agility style of combat
//      and will probably be made redundanty by the new style

int cGirls::GetCombatDamage( Girl* girl, int CombatType )
{
    int damage = 5;
    
    // Some traits help for both kinds of combat
    // There are a number of them so I set them at one point each
    // This also has the effect that some traits actually do something in the
    // game now
    
    if( girl->has_trait( "Psychic" ) )
        damage++;
        
    if( girl->has_trait( "Adventurer" ) )
        damage++;
        
    if( girl->has_trait( "Aggressive" ) )
        damage++;
        
    if( girl->has_trait( "Fearless" ) )
        damage++;
        
    if( girl->has_trait( "Yandere" ) )
        damage++;
        
    if( girl->has_trait( "Merciless" ) )
        damage++;
        
    if( girl->has_trait( "Sadistic" ) )
        damage++;
        
    if( girl->has_trait( "Twisted" ) )
        damage++;
        
    if( unsigned( CombatType ) == SKILL_MAGIC )
    {
        damage += g_Girls.GetSkill( girl, SKILL_MAGIC ) / 5 + 2;
        
        // Depending on how you see magic, charisma can influence how it flows
        // (Think Dungeons and Dragons sorcerer)
        if( girl->has_trait( "Charismatic" ) )
            damage += 1;
            
        // Same idea as charismatic.
        // Note that I love using brainwashing oil, so this hurts me more than
        // it hurts you
        if( girl->has_trait( "Iron Will" ) )
            damage += 2;
        else if( girl->has_trait( "Broken Will" ) )
            damage -= 2;
            
        if( girl->has_trait( "Strong Magic" ) )
            damage += 2;
            
        // Can Mind Fucked people even work magic?
        if( girl->has_trait( "Mind Fucked" ) )
            damage -= 5;
    }
    else   // SKILL_COMBAT case
    {
        damage += g_Girls.GetSkill( girl, SKILL_COMBAT ) / 10;
        
        if( girl->has_trait( "Manly" ) )
            damage += 2;
            
        if( girl->has_trait( "Strong" ) )
            damage += 2;
    }
    
    return damage;
}

// MYR: Separated out taking combat damage from taking damage from other sources
//     Combat damage can be lowered a bit by certain traits, where other kinds of
//     damage cannot
//
// Returns the new health value

int cGirls::TakeCombatDamage( Girl* girl, int amt )
{

    if( HasTrait( girl, "Incorporial" ) )
    {
        girl->m_Stats[STAT_HEALTH] = 100;   // WD: Sanity - Incorporial health should allways be at 100%
        return 100;                         // MYR: Sanity is good. Moved to the top
    }
    
    if( amt == 0 )
        return girl->m_Stats[STAT_HEALTH];
        
    int value;
    
    // This function works with negative numbers, but we'll be flexible and take
    // positive numbers as well
    if( amt > 0 )
        value = amt * -1;
    else
        value = amt;
        
    // High con allows you to shrug off damage
    value = value + ( GetStat( girl, STAT_CONSTITUTION ) / 20 );
    
    if( HasTrait( girl, "Fragile" ) ) // Takes more damage
        value -= 3;
    else if( HasTrait( girl, "Tough" ) ) // Takes less damage
        value += 2;
        
    if( girl->has_trait( "Adventurer" ) )
        value++;
        
    if( girl->has_trait( "Fleet of Foot" ) )
        value++;
        
    if( girl->has_trait( "Optimist" ) )
        value++;
    else if( girl->has_trait( "Pessimist" ) )
        value--;
        
    if( girl->has_trait( "Manly" ) )
        value++;
        
    if( girl->has_trait( "Maschoist" ) )
        value += 2;
        
    if( value >= 0 ) //  Can't heal when damage is dealed
        value = -1;  //  min 1 damage inflicted
        
        
    if( HasTrait( girl, "Construct" ) )
    {
        if( value < -4 )
            value = -4;
    }
    
    girl->m_Stats[STAT_HEALTH] += value;
    
    /* unsigned char cannot be < 0
    if (girl->m_Stats[STAT_HEALTH] < 0)  // Consistency?
        girl->m_Stats[STAT_HEALTH] = 0;
    */
    
    return girl->m_Stats[STAT_HEALTH];
}

// ----- Update

void cGirls::UpdateEnjoyment( Girl* girl, int whatSheEnjoys, int amount, bool wrapTo100 )
{
    girl->m_Enjoyment[whatSheEnjoys] += amount;
    
    if( wrapTo100 == true )
    {
        if( girl->m_Enjoyment[whatSheEnjoys] > 100 )
        {
            girl->m_Enjoyment[whatSheEnjoys] = 100;
        }
        else if( girl->m_Enjoyment[whatSheEnjoys] < -100 )
        {
            girl->m_Enjoyment[whatSheEnjoys] = -100;
        }
    }
}

void cGirls::updateGirlAge( Girl* girl, bool inc_inService )
{
    /*
     *  Increment birthday counter and update Girl's age if needed
     */
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    if( inc_inService )
    {
        girl->m_WeeksPast++;
        girl->m_BDay++;
    }
    
    if( girl->m_BDay >= 52 )                // Today is girl's birthday
    {
        girl->m_BDay = 0;
        girl->age( 1 );
        
        if( girl->age() > 20 && girl->has_trait( "Lolita" ) )
            g_Girls.RemoveTrait( girl, "Lolita" );
    }
}

void cGirls::updateSTD( Girl* girl )
{
    /*
     *   Update health for any STD
     */
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    // STDs
    if( girl->has_trait( "AIDS" ) )
        girl->health( -15 );
        
    if( girl->has_trait( "Chlamydia" ) )
        girl->health( -2 );
        
    if( girl->has_trait( "Syphilis" ) )
        girl->health( -5 );
        
    if( girl->health() <= 0 )
    {
        std::string msg = girl->m_Realname + " has died from STD's.";
        girl->m_Events.AddMessage( msg, IMGTYPE_DEATH, EVENT_DANGER );
        //g_MessageQue.AddToQue(msg, 1);
    }
}

void cGirls::updateGirlTurnStats( Girl* girl )
{
    /*
     *  Stat update code that is to be run every turn
     */
    
    // Sanity check. Abort on dead girl
    if( girl->health() <= 0 )
    {
        return;
    }
    
    // TIREDNESS Really tired girls get unhappy fast
    int bonus = girl->tiredness() - 90;
    
    if( bonus > 0 )
    {
        girl->obedience( -1 );              // Base loss for being tired
        girl->pclove( -1 );
        
        bonus   = bonus / 3 + 1;            // bonus vs tiredness values 1: 91-92, 2: 93-95, 3: 96-98, 4: 99-100
        girl->happiness( -bonus );
        
        bonus   = bonus / 2 + 1;            // bonus vs tiredness values 1: 91-92, 2: 93-98, 3: 99-100
        
        if( girl->health() - bonus < 10 )   // Don't kill the girl from tiredness
        {
            girl->health( 10 );             // Girl will hate player more if badly hurt from being tired
            girl->pclove( -1 );
            girl->pchate( 1 );
        }
        else
        {
            girl->health( -bonus );         // Really tired girls lose more health
        }
        
        /*      These messages duplicate warning messages in the matron code
         *
         *      msg = girlName + " is so tired her health has been affected.";
         *      girl->m_Events.AddMessage(msg, IMGTYPE_PROFILE, EVENT_WARNING);
         */
    }
    
    
    // HEALTH hurt girls get tired fast
    bonus = 40 - girl->health();
    
    if( bonus > 0 )
    {
        girl->pchate( 1 );                  // Base loss for being hurt
        girl->pclove( -1 );
        girl->happiness( -1 );
        
        bonus   = bonus / 8 + 1;            // bonus vs health values 1: 33-39, 2: 25-32, 3: 17-24, 4: 09-16 5: 01-08
        girl->tiredness( bonus );
        
        bonus   = bonus / 2 + 1;            // bonus vs health values 1: 33-39, 2: 17-32, 3: 01-16
        
        if( girl->health() - bonus < 1 )    // Don't kill the girl from low health
        {
            girl->health( 1 );              // Girl will hate player more for very low health
            girl->pclove( -1 );
            girl->pchate( 1 );
        }
        else
        {
            girl->health( -bonus );
        }
        
        /*      These messages duplicate warning messages in the matron code
         *
         *      msg = "DANGER " + girlName + " health is low!";
         *      girl->m_Events.AddMessage(msg, IMGTYPE_PROFILE, EVENT_DANGER);
         */
    }
    
    
    // LOVE love is updated only if happiness is >= 100 or < 50
    if( girl->happiness() >= 100 )
    {
        girl->pclove( 2 );                  // Happy girls love player more
    }
    
    if( !girl->is_slave() && girl->happiness() < 50 )
    {
        girl->pclove( -2 );                 // Unhappy FREE girls love player less
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

bool cGirls::CalcPregnancy( Girl* girl, int chance, int type, unsigned char stats[NUM_STATS], unsigned char skills[NUM_SKILLS] )
{
    std::string text = "she has";
    
    /*
     *  for reasons I do not understand, but nevertheless think
     *  are kind of cool, virgins have a +10 to their pregnancy
     *  chance
     */
    if( girl->m_Virgin && chance > 0 )
    {
        chance += 10;
    }
    
    /*
     *  If there's a condition that would stop her getting preggers
     *  then we get to go home early
     *
     *  return TRUE to indicate that pregnancy is FALSE
     *  (actually, supposed to mean that contraception is true,
     *  but it also applies for things like being pregnant,
     *  or just blowing the dice roll. That gets confusing too.
     */
    if( has_contraception( girl ) )
    {
        return true;
    }
    
    /*
     *  the other effective form of contraception, of course,
     *  is failing the dice roll. Let's check the chance of
     *  her NOT getting preggers here
     */
    if( g_Dice.percent( 100 - chance ) )
    {
        return true;
    }
    
    /*
     *  set the pregnant status
     */
    girl->m_States |= ( 1 << type );
    /*
     *  narrative depends on what it was that Did The Deed
     *  specifically, was it human or not?
     */
    
    switch( type )
    {
    case STATUS_INSEMINATED:
        text += " been inseminated.";
        break;
        
    case STATUS_PREGNANT:
        text += " gotten pregnant.";
        break;
        
    case STATUS_PREGNANT_BY_PLAYER:
        text += " gotten pregnant with you.";
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    girl->m_Events.AddMessage( text, IMGTYPE_PREGNANT, EVENT_DANGER );
    
    sChild* child = new sChild( unsigned( type ) == STATUS_PREGNANT_BY_PLAYER );
    
    for( int i = 0; i < NUM_STATS; i++ )
        child->m_Stats[i] = stats[i];
        
    for( u_int i = 0; i < NUM_SKILLS; i++ )
        child->m_Skills[i] = skills[i];
        
    // if there is somehow leftover pregnancy data, clear it
    girl->m_WeeksPreg = 0;
    sChild* leftover = girl->m_Children.m_FirstChild;
    
    while( leftover )
    {
        leftover = girl->next_child( leftover, ( leftover->m_Unborn > 0 ) );
    }
    
    girl->m_Children.add_child( child );
    return false;
}

int cGirls::calc_abnormal_pc( Girl* mom, Girl* sprog, bool is_players )
{
    /*
     *  the non-pc-daughter case is simpler
     */
    if( is_players == false )
    {
        /*
         *      if the mom is your daughter then any customer is
         *      a safe dad - genetically speaking, anyway
         */
        if( mom->has_trait( "Your Daughter" ) )
        {
            return 0;
        }
        
        /*
         *      so what are the odds that this customer
         *      fathered both mom and sprog. Let's say 2%
         */
        if( g_Dice.percent( 98 ) )
        {
            return 0;
        }
        
        /*
         *      that's enough to give the sprog the incest trait
         *      but there's only a risk of abnormality if
         *      mom is herself incestuous
         */
        sprog->add_trait( "Incest", false );
        
        if( mom->has_trait( "Incest" ) == false )
        {
            return 0;
        }
        
        /*
         *      If we get past all that lot, there's
         *      a 5% chance of abnormality
         */
        return 5;
    }
    
    /*
     * OK. The sprog is the player's get
     */
    sprog->add_trait( "Your Daughter", false );
    
    /*
     *  if mom isn't the player's then there is no problem
     */
    if( mom->has_trait( "Your Daughter" ) == false )
    {
        return 0;
    }
    
    /*
     *  she IS, so we add the incest trait
     */
    sprog->add_trait( "Incest", false );
    
    /*
     *  if mom is also incestuous, that adds 5% to the odds
     */
    if( mom->has_trait( "Incest" ) )
    {
        return 10;
    }
    
    return 5;
}

bool cGirls::child_is_grown( Girl* mom, sChild* child, std::string& summary, bool PlayerControlled )
{
    cConfig cfg;
    cTariff tariff;
    std::stringstream ss;
    /*
     *  bump the age - if it's still not grown, go home
     */
    child->m_Age++;
    
    if( child->m_Age < cfg.pregnancy.weeks_till_grown() )
    {
        return false;
    }
    
    /*
     *  we need a coming of age ceremony
     *
     *  for boys, slap 'em in irons and sell 'em into slavery
     */
    if( child->is_boy() )
    {
        summary += "A son grew of age. ";
        /*
         *      get the going rate for a male slave
         *      and sell the poor sod
         */
        mom->m_States |= ( 1 << STATUS_HAS_SON );
        
        if( PlayerControlled )
        {
            int gold = tariff.male_slave_sales();
            g_Gold.slave_sales( gold );
            /*
             *          format a message
             */
            ss << "Her son has grown of age and has been sold into slavery.\n";
            ss << "You make " << gold << " gold selling the boy.\n";
            /*
             *          and tell the player
             */
            mom->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_DANGER );
        }
        
        return true;
    }
    
    mom->m_States |= ( 1 << STATUS_HAS_DAUGHTER );
    bool slave = mom->is_slave();
    bool AllowNonHuman = mom->has_trait( "Not Human" );
    /*
     *  create a new girl for the bairn
     *
     *  WD: Bugfix as reported by chronos
     *      http://pinkpetal.co.cc/index.php?topic=416.msg11968#msg11968
     */
    //Girl* sprog = g_Girls.CreateRandomGirl(12, false, "", slave,AllowNonHuman);
    Girl* sprog = g_Girls.CreateRandomGirl( 12, false, slave, false, AllowNonHuman );
    /*
     *  check for incest, get the odds on abnormality
     */
    int abnormal_pc = calc_abnormal_pc( mom, sprog, ( child->m_IsPlayers != 0 ) );
    
    if( g_Dice.percent( abnormal_pc ) )
    {
        if( g_Dice.percent( 50 ) )
        {
            g_Girls.AddTrait( sprog, "Malformed" );
        }
        else
        {
            g_Girls.AddTrait( sprog, "Retarded" );
        }
    }
    
    /*
     *  loop throught the mom's traits, inheriting where appropriate
     */
//  for(int i=0; i<30; i++)     // WD: wrong loop conditions might be reason for excessive traits on kids
    for( int i = 0; i < MAXNUM_TRAITS; i++ )
    {
        if( g_Girls.InheritTrait( mom->m_Traits[i] ) )
            g_Girls.AddTrait( sprog, mom->m_Traits[i]->m_Name );
    }
    
    // inherit stats
    for( int i = 0; i < NUM_STATS; i++ )
    {
        int min, max;
        
        if( mom->m_Stats[i] < child->m_Stats[i] )
        {
            min = mom->m_Stats[i];
            max = child->m_Stats[i];
        }
        else
        {
            max = mom->m_Stats[i];
            min = child->m_Stats[i];
        }
        
        sprog->m_Stats[i] = ( g_Dice % ( max - min ) ) + min;
    }
    
    //set age to 17 fix health
    sprog->m_Stats[STAT_AGE] = 17;
    sprog->m_Stats[STAT_HEALTH] = 100;
    sprog->m_Stats[STAT_LEVEL] = 0;
    sprog->m_Stats[STAT_EXP] = 0;
    
    // inherit skills
    for( u_int i = 0; i < NUM_SKILLS; i++ )
    {
        int max;
        
        if( mom->m_Skills[i] < child->m_Skills[i] )
        {
            max = child->m_Skills[i];
            
            if( max > 20 )
                max = 20;
                
            sprog->m_Skills[i] = g_Dice % max;
        }
        else
        {
            max = mom->m_Skills[i];
            
            if( max > 20 )
                max = 20;
                
            sprog->m_Skills[i] = g_Dice % max;
        }
    }
    
    // make sure slave daughters have house perc. set to 100, otherwise 60
    if( slave )
        sprog->m_Stats[STAT_HOUSE] = cfg.initial.slave_house_perc();
    else
        sprog->m_Stats[STAT_HOUSE] = 60;
        
    if( PlayerControlled )
    {
        summary += "A daughter grew of age. ";
        mom->m_Events.AddMessage(
            "Her daughter has grown of age and has been placed in your dungeon.",
            IMGTYPE_PROFILE, EVENT_DANGER
        );
        g_Brothels.GetDungeon()->AddGirl( sprog, 13 );
    }
    else
        g_Girls.AddGirl( sprog );
        
    return true;
}

int cGirls::num_images( Girl* girl, int image_type )
{
    return girl->m_GirlImages->m_Images[image_type].m_NumImages;
}

void cGirls::UncontrolledPregnancies()
{
    Girl* current = m_Parent;
    std::string summary;
    
    while( current )
    {
        HandleChildren( current, summary, false );
        current = current->m_Next;
    }
}

void cGirls::HandleChildren( Girl* girl, std::string& summary, bool PlayerControlled )
{
    sChild* child;
    girl->m_JustGaveBirth = false;
    
    /*
     *  start by advancing pregnancy cooldown time
     */
    if( girl->m_PregCooldown > 0 )
    {
        girl->m_PregCooldown--;
    }
    
    /*
     *  now: if the girl has no children
     *  we have nothing to do
     *
     *  logically this can precede the cooldown bump
     *  since if she's on cooldown she must have
     *  given birth
     *
     *  but I guess this way offers better bugproofing
     */
    if( girl->m_Children.m_FirstChild == nullptr )
    {
        return;
    }
    
    /*
     *  loop through the girl's children,
     *  and divide them into those growing up
     *  and those still to be born
     */
    bool remove_flag;
    child = girl->m_Children.m_FirstChild;
    
    while( child )
    {
        /*
         *      if the child is yet unborn
         *      see if it is due
         */
        if( child->m_Unborn )
        {
            /*
             *          some births (monsters) we do not track to adulthood
             *          these need removing from the list
             */
            remove_flag = child_is_due( girl, child, summary, PlayerControlled );
        }
        else
        {
            /*
             *          the child has been born already
             *
             *          if it comes of age we remove it from the list
             */
            remove_flag = child_is_grown( girl, child, summary, PlayerControlled );
        }
        
        child = girl->next_child( child, remove_flag );
    }
}

bool cGirls::child_is_due( Girl* girl, sChild* child, std::string& summary, bool PlayerControlled )
{
    cConfig cfg;
    cTariff tariff;
    std::stringstream ss;
    /*
     *  clock on the count and see if she's due
     *  if not, return false (meaning "do not remove this child yet)
     */
    girl->m_WeeksPreg++;
    
    if( girl->m_WeeksPreg < cfg.pregnancy.weeks_pregnant() )
    {
        return false;
    }
    
    /*
     *  OK, it's time to give birth
     *  start with some basic bookkeeping.
     */
    girl->m_WeeksPreg = 0;
    child->m_Unborn = 0;
    girl->m_PregCooldown = cfg.pregnancy.cool_down();
    
    //ADB low health is risky for pregnancy!
    //80 health will add 2 to percent chance of sterility and death, 10 health will add 9 percent!
    int healthFactor = ( 100 - g_Girls.GetStat( girl, STAT_HEALTH ) ) / 10;
    
    /*
     *  the human-baby case is marginally easier than the
     *  tentacle-beast-monstrosity one, so we do that first
     */
    if( girl->carrying_human() )
    {
        /*
         *      first things first - clear the pregnancy bit
         *      this is a human birth, so add the MILF trait
         */
        girl->clear_pregnancy();
        g_Girls.AddTrait( girl, "MILF" );
        /*
         *      format a message
         */
        summary += "Gave birth. ";
        ss << "She has given birth to a baby "
           << child->boy_girl_str()
           << ". You grant her the week off for maternity leave."
           ;
           
        /*
         *      check for sterility
         */
        if( g_Dice.percent( 1 + healthFactor ) )
        {
            ss << " It was a difficult birth and she has lost the ability to have children.";
            g_Girls.AddTrait( girl, "Sterile" );
        }
        
        /*
         *      queue the message and return false because we need to
         *      see this one grow up
         */
        if( PlayerControlled )
            girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_DANGER );
            
        girl->m_JustGaveBirth = true;
        return false;
    }
    
    /*
     *  It's monster time!
     *
     *  How much does one of these suckers bring on the open market
     *  anyway?
     *
     *  might as well record the transaction and clear the preggo bit
     *  while we're at it
     */
    if( PlayerControlled )
    {
        long gold = tariff.creature_sales();
        g_Gold.creature_sales( gold );
        
        summary += "Gave birth to a beast. ";
        ss << "The creature within her has matured and emerged from her womb.\n";
        ss << "You make " << gold << " gold selling the creature.";
        ss << " You grant her the week off for her body to recover.";
    }
    
    girl->clear_pregnancy();
    
    /*
     *  check for death
     */
    if( g_Dice.percent( 1 + healthFactor ) )
    {
        summary += "And died from it. ";
        ss << "\nSadly, the girl did not survive the experience.";
        girl->m_Stats[STAT_HEALTH] = 0;
    }
    /*
     *  and sterility
     *  slight mod: 1% death, 5% sterility for monster births
     *  as opposed to other way around. Seems better this way.
     */
    else if( g_Dice.percent( 5 + healthFactor ) )
    {
        ss << " It was a difficult birth and she has lost the ability to have children.";
        g_Girls.AddTrait( girl, "Sterile" );
    }
    
    /*
     *  queue the message and return TRUE
     *  because we're not interested in watching
     *  little tentacles grow to adulthood
     */
    if( PlayerControlled )
        girl->m_Events.AddMessage( ss.str(), IMGTYPE_PROFILE, EVENT_DANGER );
        
    girl->m_JustGaveBirth = true;
    return true;
}

bool cGirls::InheritTrait( sTrait* trait )
{

    /*
     * WD: tidy up and add default chance for unknown traits to inherit
     *
     */
    
    if( trait )
    {
        if( strcmp( trait->m_Name, "Fragile" ) == 0       ||
                strcmp( trait->m_Name, "Fleet of Foot" ) == 0 ||
                strcmp( trait->m_Name, "Clumsy" ) == 0        ||
                strcmp( trait->m_Name, "Strong" ) == 0        ||
                strcmp( trait->m_Name, "Psychic" ) == 0       ||
                strcmp( trait->m_Name, "Strong Magic" ) == 0
          )
        {
            if( g_Dice.percent( 30 ) )
                return true;
        }
        
        if( strcmp( trait->m_Name, "Tough" ) == 0         ||
                strcmp( trait->m_Name, "Fast orgasms" ) == 0  ||
                strcmp( trait->m_Name, "Slow orgasms" ) == 0  ||
                strcmp( trait->m_Name, "Quick Learner" ) == 0 ||
                strcmp( trait->m_Name, "Slow Learner" ) == 0
          )
        {
            if( g_Dice.percent( 50 ) )
                return true;
        }
        
        if( strcmp( trait->m_Name, "Perky Nipples" ) == 0 ||
                strcmp( trait->m_Name, "Puffy Nipples" ) == 0 ||
                strcmp( trait->m_Name, "Long Legs" ) == 0     ||
                strcmp( trait->m_Name, "Big Boobs" ) == 0     ||
                strcmp( trait->m_Name, "Abnormally Large Boobs" ) == 0 ||
                strcmp( trait->m_Name, "Small Boobs" ) == 0   ||
                strcmp( trait->m_Name, "Great Arse" ) == 0    ||
                strcmp( trait->m_Name, "Great Figure" ) == 0  ||
                strcmp( trait->m_Name, "Cute" ) == 0
          )
        {
            if( g_Dice.percent( 70 ) )
                return true;
        }
        
        if( strcmp( trait->m_Name, "Demon" ) == 0         ||
                strcmp( trait->m_Name, "Cat Girl" ) == 0      ||
                strcmp( trait->m_Name, "Not Human" ) == 0     ||
                strcmp( trait->m_Name, "Shroud Addict" ) == 0 ||
                strcmp( trait->m_Name, "Fairy Dust Addict" ) == 0 ||
                strcmp( trait->m_Name, "Viras Blood Addict" ) == 0
          )
            return true;
            
        if( strcmp( trait->m_Name, "Nymphomaniac" ) == 0 )
        {
            if( g_Dice.percent( 60 ) )
                return true;
        }
        
        if( strcmp( trait->m_Name, "Strange Eyes" ) == 0 ||
                strcmp( trait->m_Name, "Different Colored Eyes" ) == 0
          )
        {
            if( g_Dice.percent( 70 ) )
                return true;
        }
        
        // WD: traits that can't be inherited or are a special case
        if( strcmp( trait->m_Name, "Construct" ) == 0     ||
                strcmp( trait->m_Name, "Half-Construct" ) == 0 ||
                strcmp( trait->m_Name, "Cool Scars" ) == 0    ||
                strcmp( trait->m_Name, "Small Scars" ) == 0   ||
                strcmp( trait->m_Name, "Horrific Scars" ) == 0 ||
                strcmp( trait->m_Name, "MILF" ) == 0          ||
                strcmp( trait->m_Name, "Your Daughter" ) == 0 ||
                strcmp( trait->m_Name, "Incest" ) == 0        ||
                strcmp( trait->m_Name, "One Eye" ) == 0       ||
                strcmp( trait->m_Name, "Eye Patch" ) == 0     ||
                strcmp( trait->m_Name, "Retarded" ) == 0      ||
                strcmp( trait->m_Name, "AIDS" ) == 0          ||
                strcmp( trait->m_Name, "Malformed" ) == 0     ||
                strcmp( trait->m_Name, "Chlamydia" ) == 0     ||
                strcmp( trait->m_Name, "Syphilis" ) == 0      ||
                strcmp( trait->m_Name, "Assassin" ) == 0      ||
                strcmp( trait->m_Name, "Adventurer" ) == 0    ||
                strcmp( trait->m_Name, "Mind Fucked" ) == 0   ||
                strcmp( trait->m_Name, "Broken Will" ) == 0   ||
                strcmp( trait->m_Name, "Sterile" ) == 0
          )
            return false;
            
        // WD: Any unlisted traits here
        if( g_Dice.percent( 30 ) )
            return true;
    }
    
    return false;
}

#if 0
void cGirls::HandleChildren( Girl* girl, std::string summary )
{
    cConfig cfg;
    
    if( girl->m_PregCooldown > 0 )
        girl->m_PregCooldown--;
        
    if( girl->m_Children.m_FirstChild )
    {
        sChild* child = girl->m_Children.m_FirstChild;
        
        while( child )
        {
            if( child->m_Unborn == 0 )
                else    // handle the pregnancy
                {
                    girl->m_WeeksPreg++;
                    
                    if( girl->m_WeeksPreg >= cfg.pregnancy.weeks_pregnant() )
                    {
                        girl->m_PregCooldown = cfg.pregnancy.cool_down();
                        
                        if( girl->m_States & ( 1 << STATUS_INSEMINATED ) )
                            g_Girls.AddTrait( girl, "MILF" );
                            
                        std::string message = "";
                        
                        if( girl->m_States & ( 1 << STATUS_PREGNANT ) || girl->m_States & ( 1 << STATUS_PREGNANT_BY_PLAYER ) )
                        {
                            summary += "Gave birth. ";
                            message = "She has given birth to a baby ";
                            
                            if( child->m_Sex == 1 )
                                message += "boy.";
                            else
                                message += "girl.";
                                
                            girl->m_States &= ~( 1 << STATUS_PREGNANT );
                            girl->m_States &= ~( 1 << STATUS_PREGNANT_BY_PLAYER );
                            
                            if( g_Dice % 100 == 0 )
                            {
                                message += " It was a difficult birth and she has lost the ability to have children.";
                                g_Girls.AddTrait( girl, "Sterile" );
                            }
                        }
                        else if( girl->m_States & ( 1 << STATUS_INSEMINATED ) )
                        {
                            summary += "Gave birth to beast. ";
                            message = "The creature within her has matured and emerged from her womb.\n";
                            message += "You make ";
                            long gold = ( g_Dice % 2000 ) + 100;
                            _ltoa( gold, buffer, 10 );
                            message += buffer;
                            message += " gold selling the creature.\n";
                            g_Gold.creature_sales( gold );
                            
                            int death = g_Dice % 101;
                            
                            if( death < 5 )
                            {
                                summary += "And died from it. ";
                                message += "Sadly the girl did not survive the experiance.";
                                girl->m_Stats[STAT_HEALTH] = 0;
                            }
                            
                            girl->m_States &= ~( 1 << STATUS_INSEMINATED );
                            
                            if( g_Dice % 100 == 0 )
                            {
                                message += " It was a difficult birth and she has lost the ability to have children.";
                                g_Girls.AddTrait( girl, "Sterile" );
                            }
                            
                            girl->m_Events.AddMessage( message, IMGTYPE_PREGNANT, 3 );
                            girl->m_WeeksPreg = 0;
                            child->m_Unborn = 0;
                            
                            // remove from the list
                            //child = girl->m_Children.remove_child(child,girl);
                            continue;
                        }
                        
                        girl->m_Events.AddMessage( message, IMGTYPE_PREGNANT, 3 );
                        girl->m_WeeksPreg = 0;
                        child->m_Unborn = 0;
                    }
                }
                
            child = child->m_Next;
        }
    }
}
#endif

// ----- Image

void cGirls::LoadGirlImages( Girl* girl )
{
    girl->m_GirlImages = g_Girls.GetImgManager()->LoadList( girl->m_Name );
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

void cGirls::LoadDefaultImages()    // for now they are hard coded
{
    m_DefImages = m_ImgListManager.LoadList( "Default" );
}

bool cGirls::IsAnimatedSurface( Girl* girl, int ImgType, int& img )
{
    while( 1 )
    {
        switch( ImgType )
        {
        case IMGTYPE_ANAL:
            if( girl->m_GirlImages->m_Images[IMGTYPE_ANAL].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_ANAL].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_BDSM:
            if( girl->m_GirlImages->m_Images[IMGTYPE_BDSM].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BDSM].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_BEAST:
            if( girl->m_GirlImages->m_Images[IMGTYPE_BEAST].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BEAST].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_GROUP:
            if( girl->m_GirlImages->m_Images[IMGTYPE_GROUP].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_GROUP].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_LESBIAN:
            if( girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_PREGNANT:
            if( girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PREGNANT].IsAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_DEATH:
            if( girl->m_GirlImages->m_Images[IMGTYPE_DEATH].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_DEATH].IsAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_DEATH].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_PROFILE:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages )
                ImgType = IMGTYPE_PREGNANT;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PROFILE].IsAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].IsAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_SEX:
            if( girl->m_GirlImages->m_Images[IMGTYPE_SEX].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_SEX].IsAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_SEX].IsAnimatedSurface( img );
                
            break;
            
        default:
        
            g_LogFile.ss()  << "cGirls::IsAnimatedSurface: "
                    << "unexpected image type: "
                    << ImgType
                    ;
            g_LogFile.ssend();
            break;
        }
    }
    
    return 0;
}

std::shared_ptr<CSurface> cGirls::GetImageSurface( Girl* girl, int ImgType, bool random, int& img, bool gallery )
{
    while( 1 )
    {
        /*
         *      if you sell a girl from the dungeon, and then hotkey back to girl management
         *      it crashes with girl->m_GirlImages == 0
         *
         *      so let's test for that here
         */
        if( !girl || !girl->m_GirlImages )
        {
            break;
        }
        
        switch( ImgType )
        {
        case IMGTYPE_ANAL:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGANAL].m_NumImages && !gallery )
                ImgType = IMGTYPE_PREGANAL;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_ANAL].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_ANAL].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_BDSM:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGBDSM].m_NumImages && !gallery )
                ImgType = IMGTYPE_PREGBDSM;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_BDSM].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BDSM].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_BEAST:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGBEAST].m_NumImages && !gallery )
                ImgType = IMGTYPE_PREGBEAST;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_BEAST].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BEAST].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_GROUP:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGGROUP].m_NumImages && !gallery )
                ImgType = IMGTYPE_PREGGROUP;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_GROUP].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_GROUP].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_LESBIAN:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGLESBIAN].m_NumImages && !gallery )
                ImgType = IMGTYPE_PREGLESBIAN;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_PREGNANT:
            if( girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PREGNANT].GetImageSurface( random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_DEATH:
            if( girl->m_GirlImages->m_Images[IMGTYPE_DEATH].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_DEATH].GetImageSurface( random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_DEATH].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_PROFILE:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages && !gallery )
                ImgType = IMGTYPE_PREGNANT;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PROFILE].GetImageSurface( random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_PREGANAL:
        case IMGTYPE_PREGBDSM:
        case IMGTYPE_PREGBEAST:
        case IMGTYPE_PREGGROUP:
        case IMGTYPE_PREGLESBIAN:
        case IMGTYPE_PREGSEX:
            if( girl->m_GirlImages->m_Images[ImgType].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[ImgType].GetImageSurface( random, img );
                
            break;
            
        case IMGTYPE_SEX:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGSEX].m_NumImages && !gallery )
                ImgType = IMGTYPE_PREGSEX;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_SEX].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_SEX].GetImageSurface( random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_SEX].GetImageSurface( random, img );
                
            break;
            
        default:
            //error!
            break;
        }
    }
    
    return nullptr;
}

cAnimatedSurface* cGirls::GetAnimatedSurface( Girl* girl, int ImgType, int& img )
{
    return girl->m_GirlImages->m_Images[ImgType].GetAnimatedSurface( img );
    
#if 0    // the function should only be called after image is already verified as animated, so none of this below should be necessary
    
    while( 1 )
    {
        switch( ImgType )
        {
        case IMGTYPE_ANAL:
            if( girl->m_GirlImages->m_Images[IMGTYPE_ANAL].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_ANAL].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_BDSM:
            if( girl->m_GirlImages->m_Images[IMGTYPE_BDSM].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BDSM].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_BEAST:
            if( girl->m_GirlImages->m_Images[IMGTYPE_BEAST].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BEAST].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_GROUP:
            if( girl->m_GirlImages->m_Images[IMGTYPE_GROUP].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_GROUP].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_LESBIAN:
            if( girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_PREGNANT:
            if( girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PREGNANT].GetAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_DEATH:
            if( girl->m_GirlImages->m_Images[IMGTYPE_DEATH].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_DEATH].GetAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_DEATH].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_PROFILE:
            if( girl->is_pregnant() && girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages )
                ImgType = IMGTYPE_PREGNANT;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PROFILE].GetAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].GetAnimatedSurface( img );
                
            break;
            
        case IMGTYPE_SEX:
            if( girl->m_GirlImages->m_Images[IMGTYPE_SEX].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_SEX].GetAnimatedSurface( img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_SEX].GetAnimatedSurface( img );
                
            break;
        }
    }
    
    return 0;
#endif
}

/*
 * takes a girl, and image type number, and the pregnant equivalent thereof.
 *
 * returns the pregnant number if A) girl is preggers and B) girl specific images
 * exist
 *
 * Otherwise returns the non-preggy number, if the girl has images for that action
 *
 * if not, returns -1 so the caller can sort it out
 */
int cGirls::get_modified_image_type( Girl* girl, int image_type, int preg_type )
{
    bool preg = girl->is_pregnant();
    
    /*
     *  if she's pregnant, and if there exist pregnant images for
     *  whatever this action is - use them
     */
    if( preg && girl->m_GirlImages->m_Images[preg_type].m_NumImages > 0 )
    {
        return preg_type;
    }
    
    /*
     *  if not, pregnant or not, try and find a non pregnant image for this sex type
     *  the alternative would be to use a pregnant vanilla sex image - but
     *  we're keeping the sex type in preference to the pregnancy
     */
    if( girl->m_GirlImages->m_Images[image_type].m_NumImages > 0 )
    {
        return image_type;
    }
    
    /*
     *  rather than try for pregnant straight sex and straight sex
     *  let's just return -1 here and let the caller re-try with
     *  normal sex arguments
     */
    return -1;
}

/*
 * Given an image type, this tries to draw the girls own version
 * of that image if available. If not, it takes one from the default
 * set
 */
int cGirls::draw_with_default(
    Girl* girl,
    int x, int y,
    int width, int height,
    int ImgType,
    bool random,
    int img
)
{
    cImageList* images;
    
    /*
     *  does the girl have her own pics for this image type
     *  or do we need to use the default ones?
     */
    if( girl->m_GirlImages->m_Images[ImgType].m_NumImages == 0 )
    {
        images = m_DefImages->m_Images + ImgType;
    }
    else
    {
        images = girl->m_GirlImages->m_Images + ImgType;
    }
    
    /*
     *  draw and return
     */
    return images->DrawImage( x, y, width, height, random, img );
}

int cGirls::DrawGirl( Girl* girl, int x, int y, int width, int height, int ImgType, bool random, int img )
{
    bool preg = false;
    
    if( girl->is_pregnant() )
        preg = true;
        
    while( 1 )
    {
        switch( ImgType )
        {
        case IMGTYPE_ANAL:
            if( preg )
            {
                if( girl->m_GirlImages->m_Images[IMGTYPE_PREGANAL].m_NumImages != 0 )
                    return girl->m_GirlImages->m_Images[IMGTYPE_PREGANAL].DrawImage( x, y, width, height, random, img );
            }
            
            if( girl->m_GirlImages->m_Images[IMGTYPE_ANAL].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_ANAL].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_BDSM:
            if( preg )
            {
                if( girl->m_GirlImages->m_Images[IMGTYPE_PREGBDSM].m_NumImages != 0 )
                    return girl->m_GirlImages->m_Images[IMGTYPE_PREGBDSM].DrawImage( x, y, width, height, random, img );
            }
            
            if( girl->m_GirlImages->m_Images[IMGTYPE_BDSM].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BDSM].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_BEAST:
            if( preg )
            {
                if( girl->m_GirlImages->m_Images[IMGTYPE_PREGBEAST].m_NumImages != 0 )
                    return girl->m_GirlImages->m_Images[IMGTYPE_PREGBEAST].DrawImage( x, y, width, height, random, img );
            }
            
            if( girl->m_GirlImages->m_Images[IMGTYPE_BEAST].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_BEAST].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_GROUP:
            if( preg )
            {
                if( girl->m_GirlImages->m_Images[IMGTYPE_PREGGROUP].m_NumImages != 0 )
                    return girl->m_GirlImages->m_Images[IMGTYPE_PREGGROUP].DrawImage( x, y, width, height, random, img );
            }
            
            if( girl->m_GirlImages->m_Images[IMGTYPE_GROUP].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_GROUP].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_LESBIAN:
            if( preg )
            {
                if( girl->m_GirlImages->m_Images[IMGTYPE_PREGLESBIAN].m_NumImages != 0 )
                    return girl->m_GirlImages->m_Images[IMGTYPE_PREGLESBIAN].DrawImage( x, y, width, height, random, img );
            }
            
            if( girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].m_NumImages == 0 )
                ImgType = IMGTYPE_SEX;
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_PREGNANT:
            if( girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PREGNANT].DrawImage( x, y, width, height, random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_DEATH:
            if( girl->m_GirlImages->m_Images[IMGTYPE_DEATH].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_DEATH].DrawImage( x, y, width, height, random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_DEATH].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_PROFILE:
            if( preg && girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages )
                ImgType = IMGTYPE_PREGNANT;
            else if( girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_PROFILE].DrawImage( x, y, width, height, random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].DrawImage( x, y, width, height, random, img );
                
            break;
            
        case IMGTYPE_SEX:
            if( preg )
            {
                if( girl->m_GirlImages->m_Images[IMGTYPE_PREGSEX].m_NumImages != 0 )
                    return girl->m_GirlImages->m_Images[IMGTYPE_PREGSEX].DrawImage( x, y, width, height, random, img );
            }
            
            if( girl->m_GirlImages->m_Images[IMGTYPE_SEX].m_NumImages == 0 )
                return m_DefImages->m_Images[IMGTYPE_SEX].DrawImage( x, y, width, height, random, img );
            else
                return girl->m_GirlImages->m_Images[IMGTYPE_SEX].DrawImage( x, y, width, height, random, img );
                
            break;
            
        default:
            std::stringstream local_ss;
            local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
            g_LogFile.write( local_ss.str() );
            break;
        }
    }
    
    return -1;
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
