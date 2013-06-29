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
#ifndef CGAMESCRIPT_H_INCLUDED_1530
#define CGAMESCRIPT_H_INCLUDED_1530
#pragma once

#include "cScripts.h" // required inheritance

namespace WhoreMasterRenewal
{

class Girl;

const int NUMVARS = 20;

class cGameScript : public cScript
{
public:
    cGameScript();
    virtual ~cGameScript();
    
    cGameScript( const cGameScript& ) = delete;
    cGameScript& operator = ( const cGameScript& ) = delete;
    
    bool Prepare( Girl* girlTarget );
    bool Release();
    
    void RunScript();   // runs the currently loaded script
    bool IsActive() { return m_Active; }
    
private:
    // The script function prototypes
    sScript* Script_Dialog( sScript* );
    sScript* Script_Init( sScript* );
    sScript* Script_EndInit( sScript* );
    sScript* Script_EndScript( sScript* );
    sScript* Script_Restart( sScript* );
    sScript* Script_ChoiceBox( sScript* );
    sScript* Script_SetVar( sScript* );
    sScript* Script_SetVarRandom( sScript* );
    sScript* Script_IfVar( sScript* );
    sScript* Script_Else( sScript* );
    sScript* Script_EndIf( sScript* );
    sScript* Script_ActivateChoice( sScript* );
    sScript* Script_IfChoice( sScript* );
    sScript* Script_SetPlayerSuspision( sScript* );
    sScript* Script_SetPlayerDisposition( sScript* );
    sScript* Script_ClearGlobalFlag( sScript* );
    sScript* Script_AddCustToDungeon( sScript* );
    sScript* Script_AddRandomGirlToDungeon( sScript* );
    sScript* Script_SetGlobal( sScript* );
    sScript* Script_SetGirlFlag( sScript* );
    sScript* Script_AddRandomValueToGold( sScript* );
    sScript* Script_AddManyRandomGirlsToDungeon( sScript* );
    sScript* Script_AddTargetGirl( sScript* );
    sScript* Script_AdjustTargetGirlStat( sScript* );
    sScript* Script_PlayerRapeTargetGirl( sScript* );
    sScript* Script_GivePlayerRandomSpecialItem( sScript* );
    sScript* Script_IfPassSkillCheck( sScript* );
    sScript* Script_IfPassStatCheck( sScript* );
    sScript* Script_IfGirlFlag( sScript* );
    sScript* Script_GameOver( sScript* );
    sScript* Script_IfGirlStat( sScript* );
    sScript* Script_IfGirlSkill( sScript* );
    sScript* Script_IfHasTrait( sScript* );
    sScript* Script_TortureTarget( sScript* );
    sScript* Script_ScoldTarget( sScript* );
    sScript* Script_NormalSexTarget( sScript* );
    sScript* Script_BeastSexTarget( sScript* );
    sScript* Script_AnalSexTarget( sScript* );
    sScript* Script_BDSMSexTarget( sScript* );
    sScript* Script_IfNotDisobey( sScript* );
    
    // The overloaded process function
    sScript* Process( sScript* Script );
    
    bool IsIfStatement( int type );
    
    // script targets (things that the script will affect with certain commands)
    Girl* m_GirlTarget = nullptr; // if not 0 then the script is affecting a girl
    
    int m_Vars[NUMVARS]; // The scripts variables
    sScript* m_CurrPos = nullptr; // the current position within the script
    bool m_Active = false;  // keeps track of weather a script is active
    bool m_Leave = false;   // keeps track of pausing script to run game loop
    int m_NestLevel = 0;    // keeps track of what if block we are in
};

} // namespace WhoreMasterRenewal

#endif // CGAMESCRIPT_H_INCLUDED_1530
