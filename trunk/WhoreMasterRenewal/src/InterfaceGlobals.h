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

#include "cInterfaceWindow.h"
#include "cInterfaceEvent.h"
#include "cMessageBox.h"
#include "InterfaceIDs.h"
#include "cChoiceMessage.h"

#ifdef LINUX
#include "linux.h"
#endif

// Globals
extern cInterfaceEventManager g_InterfaceEvents;
extern cInterfaceWindow g_MainMenu;
extern cInterfaceWindow g_GetString;
extern cInterfaceWindow g_BrothelManagement;
extern cInterfaceWindow g_ChangeJobs;
extern cInterfaceWindow g_Turnsummary;
extern cInterfaceWindow g_Gallery;
extern cInterfaceWindow g_GetInt;
extern cInterfaceWindow g_LoadGame;
extern cInterfaceWindow g_TransferGirls;
extern cMessageBox g_MessageBox;
extern cChoiceManager g_ChoiceManager;
extern cMessageQue g_MessageQue;

void LoadInterface();	// Function for loading the interface
void FreeInterface();
void ResetInterface();

#endif // INTERFACEGLOBALS_H_INCLUDED_1505
