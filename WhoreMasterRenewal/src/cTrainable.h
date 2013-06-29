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
#ifndef CTRAINABLE_H_INCLUDED_1510
#define CTRAINABLE_H_INCLUDED_1510
#pragma once

#include <vector>
#include <string>

namespace WhoreMasterRenewal
{

class Girl;

/*
 * this represents a trainaible attribute of a girl
 *
 * currently that means skill or stat.
 */
class cTrainable
{
public:
    /*
     *  comments in the forum notwithstanding, we do too need a flag here
     *  this can't be a pure virtual, since vector tries to construct
     *  a class instance before assigning a new member to a slot
     *
     *  could try making it default to skill or stat I suppose
     *  but this is probably safer
     */
    enum AType
    {
        Stat, Skill
    } ;
    
    /*
     *  constructor - nothing fancy here
     */
    cTrainable( Girl* girl = nullptr, std::string stat_name = "", int index = 0, AType typ = AType::Stat );
    cTrainable( const cTrainable& t );
    
    virtual ~cTrainable();
    
    cTrainable& operator =( const cTrainable& t );
    
    std::string name();
    /*
     *  lost the virtual here - don't need it
     */
    int value();
    void upd( int increment );
    int gain();
    
protected:
    Girl* m_girl;
    std::string m_name;
    int m_index;
    AType m_type;
    int m_gain = 0;
};

/*
 * subclasses for stat and skill attributes
 *
 * now little more than syntactic sugar
 */
class TrainableStat : public cTrainable
{
public:
    TrainableStat( Girl* girl, std::string stat_name, int index );
};

class TrainableSkill : public cTrainable
{
public:
    TrainableSkill( Girl* girl, std::string stat_name, int index );
};

/*
 * now we can write a wrapper for a girl where all her trainable bits
 * appear to be in one array, with a single access method
 */
class TrainableGirl
{
public:
    TrainableGirl( Girl* girl );
    
    TrainableGirl( const TrainableGirl& /*toCopy*/ ) = default;
    TrainableGirl& operator = ( const TrainableGirl& ) = delete;
    
    unsigned int size() { return stats.size(); }
    /*
     *  this is useful for solo training
     */
    std::string update_random( int size = 1 );
    Girl* girl() { return m_girl; }
    
    cTrainable& operator[]( int index )
    {
        return stats[index];
    }
    
private:
    Girl* m_girl;
    std::vector<cTrainable> stats = {};
};

/*
 * we also need an idealized girl - one who combines the best attributes
 * from all the girls in the training set
 *
 * we don't need an underlying Girl for this one - she's just
 * an abstraction
 */
class IdealAttr : public cTrainable
{
public:
    IdealAttr( std::vector<TrainableGirl> set, std::string name, int attr_idx );
    int value() const { return m_value; }
    void value( int n ) { m_value = n; }
    int potential() const { return m_potential; }
    void potential( int n ) { m_potential = n; }
    int noise() const { return m_rand; }
    int attr_index() const { return m_attr_idx; }
    
private:
    int m_attr_idx;
    int m_value = 0;
    int m_potential = 0;
    int m_rand;
};

/*
 * and here's the idealized girl based on those attributes
 */
class IdealGirl
{
public:
    IdealGirl( std::vector<TrainableGirl> set );
    
    unsigned int size() { return stats.size(); }
    std::vector<int> training_indices();
    
    IdealAttr& operator[]( int index )
    {
        return stats[index];
    }
    
private:
    std::vector<IdealAttr> stats = {};
};

} // namespace WhoreMasterRenewal

#endif // CTRAINABLE_H_INCLUDED_1510
