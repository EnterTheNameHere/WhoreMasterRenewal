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
#ifndef INTERFACEPROCESSES_H_INCLUDED_1504
#define INTERFACEPROCESSES_H_INCLUDED_1504
#pragma once

#include <string>

class TiXmlHandle;

namespace WhoreMasterRenewal
{

// Moved from main.cpp
extern bool g_LeftArrow;
extern bool g_RightArrow;
extern bool g_UpArrow;
extern bool g_DownArrow;
extern bool g_EnterKey;
extern bool g_InitWin;
extern bool eventrunning;
extern int g_CurrBrothel;
extern bool g_Cheats;
extern bool g_WalkAround; // for keeping track of weather have walked around town today
extern bool g_AllTogle; // used on screens when wishing to apply something to all items
extern std::string g_ReturnText;
extern long g_IntReturn;
extern bool eventrunning;
extern int g_TalkCount;
extern bool g_GenGirls;
extern char buffer[1000];

extern void MainMenu();
extern void GetString();
extern void GetInt();
extern void NewGame();
extern void BrothelScreen();
extern void ChangeGirlJobs();
extern void Turnsummary();
extern void NextWeek();
extern void GameEvents();
extern void Gallery();
extern void SaveGameXML(std::string filename);
extern bool LoadGame(std::string directory, std::string filename);
extern bool LoadGameXML(TiXmlHandle hDoc);
//This legacy loader, and others like it, will load the game from its old format
//and the game will be saved in XML format thenafter.
//No SaveGameLegacy is necessary, and all savegame format changes
//should take place in SaveGameXML / LoadGameXML, which is extensible
//Therefore, there is a one time conversion, but saves are not invalidated
extern bool LoadGameLegacy(std::string directory, std::string filename);
extern void LoadGameScreen();
//the master file has been done away with! yay! It was only a stub anyways
//all the info is now in the XML savegame
//void SaveMasterFile(std::string filename);
extern void LoadGameInfoFiles();
extern void LoadGirlsFiles();
extern void TransferGirls();

} // namespace WhoreMasterRenewal

#endif // INTERFACEPROCESSES_H_INCLUDED_1504
