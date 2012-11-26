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

#define TIXML_USE_STL
#include "tinyxml.h"

void MainMenu();
void GetString();
void GetInt();
void NewGame();
void BrothelScreen();
void ChangeGirlJobs();
void Turnsummary();
void NextWeek();
void GameEvents();
void Gallery();
void SaveGameXML(std::string filename);
bool LoadGame(std::string directory, std::string filename);
bool LoadGameXML(TiXmlHandle hDoc);
//This legacy loader, and others like it, will load the game from its old format
//and the game will be saved in XML format thenafter.
//No SaveGameLegacy is necessary, and all savegame format changes
//should take place in SaveGameXML / LoadGameXML, which is extensible
//Therefore, there is a one time conversion, but saves are not invalidated
bool LoadGameLegacy(std::string directory, std::string filename);
void LoadGameScreen();
//the master file has been done away with! yay! It was only a stub anyways
//all the info is now in the XML savegame
//void SaveMasterFile(std::string filename);
void LoadGameInfoFiles();
void LoadGirlsFiles();
void TransferGirls();

#endif // INTERFACEPROCESSES_H_INCLUDED_1504
