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
#ifndef CRENDERQUE_H_INCLUDED_1521
#define CRENDERQUE_H_INCLUDED_1521
#pragma once

namespace WhoreMasterRenewal
{

// An object that can be added to the render que, eg a character, or a game object.
class CRenderObject
{
public:
    CRenderObject() : m_Next(nullptr) {};
    virtual ~CRenderObject() {};
    
    CRenderObject( const CRenderObject& ) = delete;
	CRenderObject& operator = ( const CRenderObject& ) = delete;
    
    virtual void Draw() {};
    
    CRenderObject* m_Next;
};

class CRenderQue
{
public:
    CRenderQue();
    ~CRenderQue();
    
    CRenderQue( const CRenderQue& ) = delete;
	CRenderQue& operator = ( const CRenderQue& ) = delete;
    
    void DrawQue();
    void ClearQue();
    
    void AddObject( CRenderObject* object );
    
private:
    CRenderObject* m_Parent;
    CRenderObject* m_Last;
};

} // namespace WhoreMasterRenewal

#endif // CRENDERQUE_H_INCLUDED_1521
