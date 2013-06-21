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

#include "cGameScript.h"
#include "cMessageBox.h"
#include "cChoiceMessage.h"
#include "Brothel.hpp"
#include "GameFlags.h"
#include "cGirls.h"
#include "GirlManager.hpp"
#include "cInventory.h"
#include "cWindowManager.h"
#include "cGangs.h"
#include "cScriptUtils.h"
#include "cGirlGangFight.h"
#include "cGirlTorture.h"
#include "cRng.h"
#include "BrothelManager.hpp"
#include "CLog.h"
#include "InterfaceProcesses.h"
#include "cInterfaceWindow.h"
#include "Girl.hpp"

#include <cstring>

namespace WhoreMasterRenewal
{

cGameScript::cGameScript() : cScript(nullptr)
{
    // Clear all internal flags to false
    for( int i = 0; i < NUMVARS; ++i )
        m_Vars[i] = 0;
}

cGameScript::~cGameScript()
{
    m_CurrPos = nullptr;
    
    if( m_ScriptParent )
        delete m_ScriptParent;
        
    m_ScriptParent = nullptr;
}

bool cGameScript::Prepare( Girl* girlTarget )
{
    m_Active = true;
    m_Leave = false;
    m_NestLevel = 0;
    
    m_GirlTarget = girlTarget;
    
    for( int i = 0; i < NUMVARS; ++i )
        m_Vars[i] = 0;
        
    // run the init portion of the script if it exists
    // MOD: docclox: 'twas crashing here with m_ScriptParent == 0
    // Delta's declared an interest in this area, so I've
    // added the following test as a temp fix
    //
    // Which may not work at all, of course, since there's
    // no reliable way to test it.
    if( m_ScriptParent == nullptr )
    {
        return true;
    }
    
    if( m_ScriptParent && m_ScriptParent->m_Type == 1 )
    {
        sScript* Ptr = m_ScriptParent;
        
        while( Ptr->m_Type != 2 )
        {
            Ptr = Process( Ptr );
        }
        
        m_CurrPos = Ptr->m_Next;    // set the start of the script to the next entry after the init phase
    }
    else
        m_CurrPos = m_ScriptParent; // begin at the start of the file
        
    return true;
}

bool cGameScript::Release()
{
    m_Active = false;
    m_CurrPos = nullptr;
    
    if( m_ScriptParent )
        delete m_ScriptParent;
        
    m_ScriptParent = nullptr;
    m_GirlTarget = nullptr;
    return true;
}

bool cGameScript::IsIfStatement( int type )
{
    if( type == 40 || type == 9 || type == 13 || type == 27 || type == 28 || type == 29 || type == 31 || type == 32 || type == 33 )
        return true;
        
    return false;
}

sScript* cGameScript::Process( sScript* Script )
{
    // Jump to function based on action type
    switch( Script->m_Type )
    {
    case 0:
        return Script_Dialog( Script );
        
    case 1:
        return Script_Init( Script );
        
    case 2:
        return Script_EndInit( Script );
        
    case 3:
        return Script_EndScript( Script );
        
    case 4:
        return Script_Restart( Script );
        
    case 5:
        return Script_ChoiceBox( Script );
        
    case 6:
        return Script->m_Next;  // the TEXT command only is used for adding choice box options, so shouldn't ever be executed
        
    case 7:
        return Script_SetVar( Script );
        
    case 8:
        return Script_SetVarRandom( Script );
        
    case 9:
        return Script_IfVar( Script );
        
    case 10:
        return Script_Else( Script );
        
    case 11:
        return Script_EndIf( Script );
        
    case 12:
        return Script_ActivateChoice( Script );
        
    case 13:
        return Script_IfChoice( Script );
        
    case 14:
        return Script_SetPlayerSuspision( Script );
        
    case 15:
        return Script_SetPlayerDisposition( Script );
        
    case 16:
        return Script_ClearGlobalFlag( Script );
        
    case 17:
        return Script_AddCustToDungeon( Script );
        
    case 18:
        return Script_AddRandomGirlToDungeon( Script );
        
    case 19:
        return Script_SetGlobal( Script );
        
    case 20:
        return Script_SetGirlFlag( Script );
        
    case 21:
        return Script_AddRandomValueToGold( Script );
        
    case 22:
        return Script_AddManyRandomGirlsToDungeon( Script );
        
    case 23:
        return Script_AddTargetGirl( Script );
        
    case 24:
        return Script_AdjustTargetGirlStat( Script );
        
    case 25:
        return Script_PlayerRapeTargetGirl( Script );
        
    case 26:
        return Script_GivePlayerRandomSpecialItem( Script );
        
    case 27:
        return Script_IfPassSkillCheck( Script );
        
    case 28:
        return Script_IfPassStatCheck( Script );
        
    case 29:
        return Script_IfGirlFlag( Script );
        
    case 30:
        return Script_GameOver( Script );
        
    case 31:
        return Script_IfGirlStat( Script );
        
    case 32:
        return Script_IfGirlSkill( Script );
        
    case 33:
        return Script_IfHasTrait( Script );
        
    case 34:
        return Script_TortureTarget( Script );
        
    case 35:
        return Script_ScoldTarget( Script );
        
    case 36:
        return Script_NormalSexTarget( Script );
        
    case 37:
        return Script_BeastSexTarget( Script );
        
    case 38:
        return Script_AnalSexTarget( Script );
        
    case 39:
        return Script_BDSMSexTarget( Script );
        
    case 40:
        return Script_IfNotDisobey( Script );
        
    default:
        std::stringstream ss;
        ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( ss.str() );
        return nullptr; // Error executing
    }
    
    return nullptr; // Error executing
}

void cGameScript::RunScript()
{
    m_Leave = false;
    m_NestLevel = 0;
    sScript* curr = m_CurrPos;
    
    // Scan through script and process functions
    while( curr != nullptr && !m_Leave && m_Active )
        curr = Process( curr );
        
    if( m_Active == false )
    {
        g_ChoiceManager.Free();
        Release();
    }
}

sScript* cGameScript::Script_Dialog( sScript* Script )
{
    g_MessageQue.AddToQue( Script->m_Entries[0].m_Text, 0 );
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_Init( sScript* Script )
{
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_EndInit( sScript* Script )
{
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_EndScript( sScript* Script )
{
    m_Active = false;
    m_Leave = true;
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_ChoiceBox( sScript* Script )
{
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    g_ChoiceManager.CreateChoiceBoxResize( value[0], value[1] );
    int id = value[0];
    int num = value[1];
    int size = 0;
    Script = Script->m_Next;
    
    for( int i = 0; i < num; i++ )
    {
        int newlen = strlen( Script->m_Entries[0].m_Text );
        
        if( newlen > size )
            size = newlen;
            
        g_ChoiceManager.AddChoice( id, Script->m_Entries[0].m_Text, i );
        Script = Script->m_Next;
    }
    
    g_ChoiceManager.BuildChoiceBox( id, size );
    
    return Script; // Go to next script action
}

sScript* cGameScript::Script_Restart( sScript* Script )
{
    m_Leave = true;
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_SetVar( sScript* Script )
{
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    // Set value
    m_Vars[value[0]] = value[1];
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_SetVarRandom( sScript* Script )
{
    int num = 0;
    int value[3];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[2] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[2] = Script->m_Entries[2].m_lValue;
        
    if( Script->m_Entries[1].m_lValue == 0 )
        num = ( g_Dice % ( value[2] + 1 ) ) + value[1];
    else
        num = ( g_Dice % ( value[2] + 1 ) ) + value[1] - 1;
        
    m_Vars[value[0]] = num;
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_IfVar( sScript* Script )
{
    bool Skipping = false; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[1] = Script->m_Entries[2].m_lValue;
        
    // See if variable matches second entry
    int sel = 0;
    
    if( Script->m_Entries[1].m_Var == 1 )
        sel = m_Vars[Script->m_Entries[1].m_Selection];
    else
        sel = Script->m_Entries[1].m_Selection;
        
    switch( sel )
    {
    case 0:
        if( m_Vars[value[0]] == value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 1:
        if( m_Vars[value[0]] < value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 2:
        if( m_Vars[value[0]] <= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 3:
        if( m_Vars[value[0]] > value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 4:
        if( m_Vars[value[0]] >= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 5:
        if( m_Vars[value[0]] != value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

sScript* cGameScript::Script_Else( sScript* Script )
{
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_EndIf( sScript* Script )
{
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_ActivateChoice( sScript* Script )
{
    g_ChoiceManager.SetActive( Script->m_Entries[0].m_lValue );
    return Script->m_Next; // Go to next script action
}

sScript* cGameScript::Script_IfChoice( sScript* Script )
{
    bool Skipping; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    // See if choice flag matches second entry
    if( g_ChoiceManager.GetChoice( value[0] ) == value[1] )
        Skipping = false;
    else
        Skipping = true;
        
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

sScript* cGameScript::Script_SetPlayerSuspision( sScript* Script )
{
    int value;
    
    if( Script->m_Entries[0].m_Var == 1 )
        value = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value = Script->m_Entries[0].m_lValue;
        
    g_Brothels.GetPlayer()->suspicion( value );
    //g_Brothels.GetPlayer()->m_Suspicion += value;
    //if(g_Brothels.GetPlayer()->m_Suspicion < -100)
    //  g_Brothels.GetPlayer()->m_Suspicion = -100;
    //if(g_Brothels.GetPlayer()->m_Suspicion > 100)
    //  g_Brothels.GetPlayer()->m_Suspicion = 100;
    
    return Script->m_Next;
}

sScript* cGameScript::Script_SetPlayerDisposition( sScript* Script )
{
    int value;
    
    if( Script->m_Entries[0].m_Var == 1 )
        value = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value = Script->m_Entries[0].m_lValue;
        
    //g_Brothels.GetPlayer()->m_Disposition += value;
    //if(g_Brothels.GetPlayer()->m_Disposition < -100)
    //  g_Brothels.GetPlayer()->m_Disposition = -100;
    //if(g_Brothels.GetPlayer()->m_Disposition > 100)
    //  g_Brothels.GetPlayer()->m_Disposition = 100;
    g_Brothels.GetPlayer()->disposition( value );
    
    return Script->m_Next;
}

sScript* cGameScript::Script_ClearGlobalFlag( sScript* Script )
{
    int value;
    
    if( Script->m_Entries[0].m_Var == 1 )
        value = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value = Script->m_Entries[0].m_lValue;
        
    ClearGameFlag( value );
    return Script->m_Next;
}

sScript* cGameScript::Script_AddCustToDungeon( sScript* Script )
{
    bool wife = false;
    int value[3];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_Selection];
    else
        value[0] = Script->m_Entries[0].m_Selection;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[2] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[2] = Script->m_Entries[2].m_lValue;
        
    if( value[2] == 1 )
        wife = true;
        
    if( value[0] == 0 )
        g_Brothels.GetDungeon()->AddCust( DUNGEON_CUSTNOPAY, value[1], wife );
    else if( value[0] == 1 )
        g_Brothels.GetDungeon()->AddCust( DUNGEON_CUSTBEATGIRL, value[1], wife );
        
    return Script->m_Next;
}

sScript* cGameScript::Script_AddRandomGirlToDungeon( sScript* Script )
{
    int value[5];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_Selection];
    else
        value[0] = Script->m_Entries[0].m_Selection;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[2] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[2] = Script->m_Entries[2].m_lValue;
        
    if( Script->m_Entries[3].m_Var == 1 )
        value[3] = m_Vars[Script->m_Entries[3].m_lValue];
    else
        value[3] = Script->m_Entries[3].m_lValue;
        
    if( Script->m_Entries[4].m_Var == 1 )
        value[4] = m_Vars[Script->m_Entries[4].m_lValue];
    else
        value[4] = Script->m_Entries[4].m_lValue;
        
    bool kidnaped = false;
    int reason = 0;
    
    if( value[0] == 0 )
    {
        kidnaped = true;
        reason = DUNGEON_GIRLKIDNAPPED;
    }
    else if( value[0] == 1 )
    {
        kidnaped = true;
        reason = DUNGEON_GIRLCAPTURED;
    }
    
    bool slave = false;
    
    if( value[3] == 1 )
        slave = true;
        
    bool allowNonHuman = false;
    
    if( value[4] == 1 )
        allowNonHuman = true;
        
    int age = 0;
    
    if( value[1] == 0 )
        age = ( g_Dice % ( value[2] + 1 ) ) + value[1];
    else
        age = ( g_Dice % ( value[2] + 1 ) ) + value[1] - 1;
        
    g_Brothels.GetDungeon()->AddGirl( g_Girls.CreateRandomGirl( age, false, "", slave, allowNonHuman, kidnaped ), reason );
    
    return Script->m_Next;
}

sScript* cGameScript::Script_SetGlobal( sScript* Script )
{
    SetGameFlag( Script->m_Entries[0].m_lValue, Script->m_Entries[1].m_lValue );
    return Script->m_Next;
}

sScript* cGameScript::Script_SetGirlFlag( sScript* Script )
{
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    if( m_GirlTarget == nullptr )
        return Script->m_Next;  // this shouldn't happen
        
    m_GirlTarget->m_Flags[value[0]] = ( unsigned char )value[1];
    return Script->m_Next;
}

sScript* cGameScript::Script_AddRandomValueToGold( sScript* Script )
{
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    long gold = 0;
    
    if( value[0] == 0 )
        gold = ( g_Dice % ( value[1] + 1 ) ) + value[0];
    else
        gold = ( g_Dice % ( value[1] + 1 ) ) + value[0] - 1;
        
    g_Gold.misc_credit( gold );
    
    return Script->m_Next;
}

sScript* cGameScript::Script_AddManyRandomGirlsToDungeon( sScript* Script )
{
    int value[6];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_Selection];
    else
        value[1] = Script->m_Entries[1].m_Selection;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[2] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[2] = Script->m_Entries[2].m_lValue;
        
    if( Script->m_Entries[3].m_Var == 1 )
        value[3] = m_Vars[Script->m_Entries[3].m_lValue];
    else
        value[3] = Script->m_Entries[3].m_lValue;
        
    if( Script->m_Entries[4].m_Var == 1 )
        value[4] = m_Vars[Script->m_Entries[4].m_lValue];
    else
        value[4] = Script->m_Entries[4].m_lValue;
        
    if( Script->m_Entries[5].m_Var == 1 )
        value[5] = m_Vars[Script->m_Entries[5].m_lValue];
    else
        value[5] = Script->m_Entries[5].m_lValue;
        
    bool kidnaped = false;
    int reason = 0;
    
    if( value[1] == 0 )
    {
        kidnaped = true;
        reason = DUNGEON_GIRLKIDNAPPED;
    }
    else if( value[1] == 1 )
    {
        kidnaped = true;
        reason = DUNGEON_GIRLCAPTURED;
    }
    
    bool slave = false;
    
    if( value[4] == 1 )
        slave = true;
        
    bool allowNonHuman = false;
    
    if( value[5] == 1 )
        allowNonHuman = true;
        
    for( int i = 0; i < value[0]; i++ )
    {
        int age = 0;
        
        if( value[2] == 0 )
            age = ( g_Dice % ( value[3] + 1 ) ) + value[2];
        else
            age = ( g_Dice % ( value[3] + 1 ) ) + value[2] - 1;
            
        g_Brothels.GetDungeon()->AddGirl( g_Girls.CreateRandomGirl( age, false, "", slave, allowNonHuman, kidnaped ), reason );
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_AddTargetGirl( sScript* Script )
{
    cScriptUtils::add_girl_to_brothel( m_GirlTarget );
    return Script->m_Next;
}

sScript* cGameScript::Script_AdjustTargetGirlStat( sScript* Script )
{
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_Selection];
    else
        value[0] = Script->m_Entries[0].m_Selection;
        
    if( Script->m_Entries[1].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[1].m_lValue];
    else
        value[1] = Script->m_Entries[1].m_lValue;
        
    if( m_GirlTarget )
    {
        if( value[0] - 22 >= 0 )
            g_Girls.UpdateSkill( m_GirlTarget, value[0] - 22, value[1] );
        else
            g_Girls.UpdateStat( m_GirlTarget, value[0], value[1] );
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_PlayerRapeTargetGirl( sScript* Script )
{
    if( m_GirlTarget == nullptr )
    {
        return Script->m_Next;
    }
    
    cPlayer* player = g_Brothels.GetPlayer();
    
    g_Girls.UpdateSkill( m_GirlTarget, SKILL_BDSM, 2 );
    g_Girls.UpdateSkill( m_GirlTarget, SKILL_ANAL, 2 );
    g_Girls.UpdateSkill( m_GirlTarget, SKILL_NORMALSEX, 2 );
    
    g_Girls.UpdateStat( m_GirlTarget, STAT_HAPPINESS, -5 );
    g_Girls.UpdateStat( m_GirlTarget, STAT_HEALTH, -10 );
    g_Girls.UpdateTempStat( m_GirlTarget, STAT_LIBIDO, 2 );
    g_Girls.UpdateStat( m_GirlTarget, STAT_CONFIDENCE, -1 );
    g_Girls.UpdateStat( m_GirlTarget, STAT_OBEDIENCE, 2 );
    g_Girls.UpdateStat( m_GirlTarget, STAT_PCFEAR, 2 );
    g_Girls.UpdateStat( m_GirlTarget, STAT_PCLOVE, -2 );
    g_Girls.UpdateStat( m_GirlTarget, STAT_PCHATE, 3 );
    
    if( ( g_Dice % 100 ) + 1 <= 2 )
        g_Girls.AddTrait( m_GirlTarget, "Broken Will" );
        
    if( m_GirlTarget->m_Virgin )
        m_GirlTarget->m_Virgin = false;
        
    bool preg = !m_GirlTarget->calc_pregnancy( player, false, 1.0 );
    
    if( preg )
    {
        g_MessageQue.AddToQue( "She has gotten pregnant", 0 );
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_GivePlayerRandomSpecialItem( sScript* Script )
{
    sInventoryItem* item = g_InvManager.GetRandomItem();
    
    while( item == nullptr )
        item = g_InvManager.GetRandomItem();
        
    bool ok = false;
    
    while( !ok )
    {
        if( item->m_Rarity >= 3 )
            ok = true;
        else
        {
            item = g_InvManager.GetRandomItem();
            
            while( item == nullptr )
                item = g_InvManager.GetRandomItem();
        }
    }
    
    int curI = g_Brothels.HasItem( item->m_Name, -1 );
    bool loop = true;
    
    while( loop )
    {
        if( curI != -1 )
        {
            if( g_Brothels.m_NumItem[curI] >= 254 )
                curI = g_Brothels.HasItem( item->m_Name, curI + 1 );
            else
                loop = false;
        }
        else
            loop = false;
    }
    
    if( g_Brothels.m_NumInventory < MAXNUM_INVENTORY || curI != -1 )
    {
        if( curI != -1 )
            g_Brothels.m_NumItem[curI]++;
        else
        {
            for( int j = 0; j < MAXNUM_INVENTORY; j++ )
            {
                if( g_Brothels.m_Inventory[j] == nullptr )
                {
                    g_Brothels.m_Inventory[j] = item;
                    g_Brothels.m_EquipedItems[j] = 0;
                    g_Brothels.m_NumInventory++;
                    g_Brothels.m_NumItem[j]++;
                    break;
                }
            }
        }
    }
    else
        g_MessageQue.AddToQue( " Your inventory is full\n", 1 );
        
    return Script->m_Next;
}

sScript* cGameScript::Script_IfPassSkillCheck( sScript* Script )
{
    bool Skipping; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    
    int value = Script->m_Entries[0].m_Selection;
    
    // See if variable matches second entry
    if( g_Dice % 101 < g_Girls.GetSkill( m_GirlTarget, value ) )
        Skipping = false;
    else
        Skipping = true;
        
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

sScript* cGameScript::Script_IfPassStatCheck( sScript* Script )
{
    bool Skipping; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    
    int value = Script->m_Entries[0].m_Selection;
    
    // See if variable matches second entry
    if( g_Dice % 101 < g_Girls.GetStat( m_GirlTarget, value ) )
        Skipping = false;
    else
        Skipping = true;
        
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

sScript* cGameScript::Script_IfGirlFlag( sScript* Script )
{
    bool Skipping = false; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[1] = Script->m_Entries[2].m_lValue;
        
    // See if variable matches second entry
    int sel = 0;
    
    if( Script->m_Entries[1].m_Var == 1 )
        sel = m_Vars[Script->m_Entries[1].m_Selection];
    else
        sel = Script->m_Entries[1].m_Selection;
        
    switch( sel )
    {
    case 0:
        if( m_GirlTarget->m_Flags[value[0]] == value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 1:
        if( m_GirlTarget->m_Flags[value[0]] < value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 2:
        if( m_GirlTarget->m_Flags[value[0]] <= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 3:
        if( m_GirlTarget->m_Flags[value[0]] > value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 4:
        if( m_GirlTarget->m_Flags[value[0]] >= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 5:
        if( m_GirlTarget->m_Flags[value[0]] != value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    default:
        std::stringstream local_ss;
        local_ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( local_ss.str() );
        break;
    }
    
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

sScript* cGameScript::Script_GameOver( sScript* Script )
{
    g_MessageQue.AddToQue( "GAME OVER", 1 );
    g_WinManager.PopToWindow( &g_BrothelManagement );
    g_WinManager.Pop();
    g_InitWin = true;
    m_Active = false;
    m_Leave = true;
    return Script->m_Next;
}

sScript* cGameScript::Script_IfGirlStat( sScript* Script )
{
    bool Skipping = false; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[1] = Script->m_Entries[2].m_lValue;
        
    // See if variable matches second entry
    int sel = 0;
    
    if( Script->m_Entries[1].m_Var == 1 )
        sel = m_Vars[Script->m_Entries[1].m_Selection];
    else
        sel = Script->m_Entries[1].m_Selection;
        
    switch( sel )
    {
    case 0:
        if( g_Girls.GetStat( m_GirlTarget, value[0] ) == value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 1:
        if( g_Girls.GetStat( m_GirlTarget, value[0] ) < value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 2:
        if( g_Girls.GetStat( m_GirlTarget, value[0] ) <= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 3:
        if( g_Girls.GetStat( m_GirlTarget, value[0] ) > value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 4:
        if( g_Girls.GetStat( m_GirlTarget, value[0] ) >= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 5:
        if( g_Girls.GetStat( m_GirlTarget, value[0] ) != value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    default:
        std::stringstream ss;
        ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( ss.str() );
        break;
    }
    
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

sScript* cGameScript::Script_IfGirlSkill( sScript* Script )
{
    bool Skipping = false; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    int value[2];
    
    if( Script->m_Entries[0].m_Var == 1 )
        value[0] = m_Vars[Script->m_Entries[0].m_lValue];
    else
        value[0] = Script->m_Entries[0].m_lValue;
        
    if( Script->m_Entries[2].m_Var == 1 )
        value[1] = m_Vars[Script->m_Entries[2].m_lValue];
    else
        value[1] = Script->m_Entries[2].m_lValue;
        
    // See if variable matches second entry
    int sel = 0;
    
    if( Script->m_Entries[1].m_Var == 1 )
        sel = m_Vars[Script->m_Entries[1].m_Selection];
    else
        sel = Script->m_Entries[1].m_Selection;
        
    switch( sel )
    {
    case 0:
        if( g_Girls.GetSkill( m_GirlTarget, value[0] ) == value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 1:
        if( g_Girls.GetSkill( m_GirlTarget, value[0] ) < value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 2:
        if( g_Girls.GetSkill( m_GirlTarget, value[0] ) <= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 3:
        if( g_Girls.GetSkill( m_GirlTarget, value[0] ) > value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 4:
        if( g_Girls.GetSkill( m_GirlTarget, value[0] ) >= value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    case 5:
        if( g_Girls.GetSkill( m_GirlTarget, value[0] ) != value[1] )
            Skipping = false;
        else
            Skipping = true;
            
        break;
        
    default:
        std::stringstream ss;
        ss << "Switch default case was hit unexpectingly.\n" << __LINE__ << ":" << __FILE__ << "\n";
        g_LogFile.write( ss.str() );
        break;
    }
    
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

sScript* cGameScript::Script_IfHasTrait( sScript* Script )
{
    bool Skipping; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    
    if( g_Girls.HasTrait( m_GirlTarget, Script->m_Entries[0].m_Text ) )
        Skipping = false;
    else
        Skipping = true;
        
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

/*
 * this ought to use common code from the dungeon screen manager
 *
 * more to the point, both of them should use code from a dungeon
 * object
 */
sScript* cGameScript::Script_TortureTarget( sScript* Script )
{
    cGirlTorture gt( m_GirlTarget );
    return Script->m_Next;
}

sScript* cGameScript::Script_ScoldTarget( sScript* Script )
{
    if( g_Girls.GetStat( m_GirlTarget, STAT_SPIRIT ) <= 10 )
    {
        g_MessageQue.AddToQue( "She is bawling the entire time you yell at her, obviously wanting to do her best", 0 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_HAPPINESS, -5 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_CONFIDENCE, -5 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_OBEDIENCE, 10 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_SPIRIT, -3 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCLOVE, -4 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCFEAR, 2 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCHATE, 2 );
    }
    else if( g_Girls.GetStat( m_GirlTarget, STAT_SPIRIT ) <= 20 )
    {
        g_MessageQue.AddToQue( "She sobs a lot while you yell at her and fearfully listens to your every word", 0 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_HAPPINESS, -2 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_CONFIDENCE, -2 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_OBEDIENCE, 6 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_SPIRIT, -2 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCLOVE, -1 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCFEAR, 1 );
    }
    else if( g_Girls.GetStat( m_GirlTarget, STAT_SPIRIT ) <= 30 )
    {
        g_MessageQue.AddToQue( "She listens with attention and promises to do better", 0 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_HAPPINESS, -1 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_CONFIDENCE, -1 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_OBEDIENCE, 5 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_SPIRIT, -2 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCLOVE, -1 );
    }
    else if( g_Girls.GetStat( m_GirlTarget, STAT_SPIRIT ) <= 50 )
    {
        g_MessageQue.AddToQue( "She listens to what you say but barely pays attention", 0 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_OBEDIENCE, 3 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_SPIRIT, -2 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCHATE, 1 );
    }
    else if( g_Girls.GetStat( m_GirlTarget, STAT_SPIRIT ) <= 80 )
    {
        g_MessageQue.AddToQue( "She looks at you defiantly while you yell at her", 0 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_OBEDIENCE, 2 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_SPIRIT, -1 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCLOVE, -3 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCHATE, 2 );
    }
    else
    {
        g_MessageQue.AddToQue( "She stares you down while you yell at her, daring you to hit her", 0 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_OBEDIENCE, -1 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_SPIRIT, -1 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCLOVE, -4 );
        g_Girls.UpdateStat( m_GirlTarget, STAT_PCHATE, 5 );
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_NormalSexTarget( sScript* Script )
{
    if( m_GirlTarget )
    {
        g_Girls.UpdateSkill( m_GirlTarget, SKILL_NORMALSEX, 2 );
        
        if( m_GirlTarget->m_Virgin )
            m_GirlTarget->m_Virgin = false;
            
        if( !m_GirlTarget->calc_pregnancy( g_Brothels.GetPlayer(), false, 1.0 ) )
        {
            g_MessageQue.AddToQue( "She has gotten pregnant", 0 );
        }
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_BeastSexTarget( sScript* Script )
{
    if( m_GirlTarget )
    {
        g_Girls.UpdateSkill( m_GirlTarget, SKILL_BEASTIALITY, 2 );
        
        if( m_GirlTarget->m_Virgin )
            m_GirlTarget->m_Virgin = false;
            
        // mod: added check for number of beasts owned; otherwise, fake beasts could somehow inseminate the girl
        if( g_Brothels.GetNumBeasts() > 0 )
        {
            if( !m_GirlTarget->calc_insemination( g_Brothels.GetPlayer(), false, 1.0 ) )
                g_MessageQue.AddToQue( "She has gotten inseminated", 0 );
        }
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_AnalSexTarget( sScript* Script )
{
    if( m_GirlTarget )
    {
        g_Girls.UpdateSkill( m_GirlTarget, SKILL_ANAL, 2 );
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_BDSMSexTarget( sScript* Script )
{
    if( m_GirlTarget )
    {
        g_Girls.UpdateSkill( m_GirlTarget, SKILL_BDSM, 2 );
        
        if( m_GirlTarget->m_Virgin )
            m_GirlTarget->m_Virgin = false;
    }
    
    if( !m_GirlTarget->calc_pregnancy( g_Brothels.GetPlayer(), false, 0.75 ) )
    {
        g_MessageQue.AddToQue( "She has gotten pregnant", 0 );
    }
    
    return Script->m_Next;
}

sScript* cGameScript::Script_IfNotDisobey( sScript* Script )
{
    bool Skipping; // Flag for if...then condition
    m_NestLevel++;
    int Nest = m_NestLevel;
    
    // See if choice flag matches second entry
    if( !g_Girls.DisobeyCheck( m_GirlTarget, ACTION_GENERAL, g_Brothels.GetBrothel( g_CurrBrothel ) ) )
        Skipping = false;
    else
        Skipping = true;
        
    // At this point, Skipping states if the script actions
    // need to be skipped due to a conditional if...then statement.
    // Actions are further processed if skipped = false, looking
    // for an else to flip the skip mode, or an endif to end
    // the conditional block.
    Script = Script->m_Next; // Go to next action to process
    
    while( Script != nullptr )
    {
        if( m_Leave )
            break;
            
        // if else, flip skip mode
        if( Script->m_Type == 10 )
        {
            if( Nest == m_NestLevel )
                Skipping = !Skipping;
        }
        
        // break on end if
        if( Script->m_Type == 11 )
        {
            if( Nest == m_NestLevel )
            {
                m_NestLevel--;
                return Script->m_Next;
            }
            
            m_NestLevel--;
        }
        
        // Process script function in conditional block
        // making sure to skip actions when condition not met.
        if( Skipping )
        {
            if( IsIfStatement( Script->m_Type ) )
                m_NestLevel++;
                
            Script = Script->m_Next;
        }
        else
        {
            if( ( Script = Process( Script ) ) == nullptr )
                return nullptr;
        }
    }
    
    return nullptr; // End of script reached
}

} // namespace WhoreMasterRenewal
