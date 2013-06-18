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
#ifndef CTIMER_H_INCLUDED_1510
#define CTIMER_H_INCLUDED_1510
#pragma once

namespace WhoreMasterRenewal
{

// Frame Rate for games
const int FRAMES_PER_SECOND = 25;

class CTimer
{
public:
	CTimer();
	~CTimer();

	void Start();
	void Stop();
	void Pause(bool pause);
	int GetTicks();
	bool IsStarted();
	bool IsPaused();

private:
	int m_StartTicks = 0;
	int m_PausedTicks = 0;
	bool m_Paused = false;
	bool m_Started = false;
};

} // namespace WhoreMasterRenewal

#endif // CTIMER_H_INCLUDED_1510
