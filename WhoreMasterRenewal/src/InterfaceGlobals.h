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
#ifndef INTERFACEGLOBALS_H_INCLUDED_1505
#define INTERFACEGLOBALS_H_INCLUDED_1505
#pragma once

#include <vector>
#include <memory>

namespace WhoreMasterRenewal
{

extern unsigned char g_MessageBoxBorderR, g_MessageBoxBorderG, g_MessageBoxBorderB;
extern unsigned char g_MessageBoxBackground0R, g_MessageBoxBackground0G, g_MessageBoxBackground0B;
extern unsigned char g_MessageBoxBackground1R, g_MessageBoxBackground1G, g_MessageBoxBackground1B;
extern unsigned char g_MessageBoxBackground2R, g_MessageBoxBackground2G, g_MessageBoxBackground2B;
extern unsigned char g_MessageBoxBackground3R, g_MessageBoxBackground3G, g_MessageBoxBackground3B;
extern unsigned char g_MessageBoxTextR, g_MessageBoxTextG, g_MessageBoxTextB;

extern unsigned char g_ListBoxBorderR, g_ListBoxBorderG, g_ListBoxBorderB;
extern unsigned char g_ListBoxBackgroundR, g_ListBoxBackgroundG, g_ListBoxBackgroundB;
extern unsigned char g_ListBoxElementBackgroundR, g_ListBoxElementBackgroundG, g_ListBoxElementBackgroundB;
extern unsigned char g_ListBoxS1ElementBackgroundR, g_ListBoxS1ElementBackgroundG, g_ListBoxS1ElementBackgroundB;
extern unsigned char g_ListBoxS2ElementBackgroundR, g_ListBoxS2ElementBackgroundG, g_ListBoxS2ElementBackgroundB;
extern unsigned char g_ListBoxSelectedElementR, g_ListBoxSelectedElementG, g_ListBoxSelectedElementB;
extern unsigned char g_ListBoxSelectedS1ElementR, g_ListBoxSelectedS1ElementG, g_ListBoxSelectedS1ElementB;
extern unsigned char g_ListBoxSelectedS2ElementR, g_ListBoxSelectedS2ElementG, g_ListBoxSelectedS2ElementB;
extern unsigned char g_ListBoxElementBorderR, g_ListBoxElementBorderG, g_ListBoxElementBorderB;
extern unsigned char g_ListBoxElementBorderHR, g_ListBoxElementBorderHG, g_ListBoxElementBorderHB;
extern unsigned char g_ListBoxTextR, g_ListBoxTextG, g_ListBoxTextB;
extern unsigned char g_ListBoxHeaderBackgroundR, g_ListBoxHeaderBackgroundG, g_ListBoxHeaderBackgroundB;
extern unsigned char g_ListBoxHeaderBorderR, g_ListBoxHeaderBorderG, g_ListBoxHeaderBorderB;
extern unsigned char g_ListBoxHeaderBorderHR, g_ListBoxHeaderBorderHG, g_ListBoxHeaderBorderHB;
extern unsigned char g_ListBoxHeaderTextR, g_ListBoxHeaderTextG, g_ListBoxHeaderTextB;

extern unsigned char g_WindowBorderR, g_WindowBorderG, g_WindowBorderB;
extern unsigned char g_WindowBackgroundR, g_WindowBackgroundG, g_WindowBackgroundB;

extern unsigned char g_EditBoxBorderR, g_EditBoxBorderG, g_EditBoxBorderB;
extern unsigned char g_EditBoxBackgroundR, g_EditBoxBackgroundG, g_EditBoxBackgroundB;
extern unsigned char g_EditBoxSelectedR, g_EditBoxSelectedG, g_EditBoxSelectedB;
extern unsigned char g_EditBoxTextR, g_EditBoxTextG, g_EditBoxTextB;

extern unsigned char g_ChoiceMessageTextR, g_ChoiceMessageTextG, g_ChoiceMessageTextB;
extern unsigned char g_ChoiceMessageBorderR, g_ChoiceMessageBorderG, g_ChoiceMessageBorderB;
extern unsigned char g_ChoiceMessageBackgroundR, g_ChoiceMessageBackgroundG, g_ChoiceMessageBackgroundB;
extern unsigned char g_ChoiceMessageSelectedR, g_ChoiceMessageSelectedG, g_ChoiceMessageSelectedB;

extern unsigned char g_CheckBoxBorderR, g_CheckBoxBorderG, g_CheckBoxBorderB;
extern unsigned char g_CheckBoxBackgroundR, g_CheckBoxBackgroundG, g_CheckBoxBackgroundB;


// Moved from main.cpp
extern bool g_ShiftDown;
extern bool g_CTRLDown;
extern int g_ScreenWidth;
extern int g_ScreenHeight;
extern bool g_Fullscreen;

extern unsigned long g_Year;
extern unsigned long g_Month;
extern unsigned long g_Day;


void LoadInterface();	// Function for loading the interface
void FreeInterface();
void ResetInterface();

} // namespace WhoreMasterRenewal

#endif // INTERFACEGLOBALS_H_INCLUDED_1505
