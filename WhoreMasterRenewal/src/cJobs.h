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
#ifndef CJOBS_H_INCLUDED_1524
#define CJOBS_H_INCLUDED_1524
#pragma once

#include <string>
#include <vector>

namespace WhoreMasterRenewal
{

class Girl;
class Brothel;
struct sGang;

typedef unsigned int u_int;

struct sFilm
{
	//int total_girls;
	//int total_cost;
	float quality_multiplyer;
	std::vector<int> scene_quality;
	char time;
	int final_quality;
	bool sex_acts_flags[5];
	int total_customers;
};

struct sJobs
{
    sJobs();
	~sJobs();
	void setup( std::string name,
                int min = 0,
                int max = -1,
                bool slave = true,
                bool free = true,
                char girlpay = 0,
                char up = 0,
                int id = 0,
                std::string like = "",
                std::string hate = "",
                int skill = 0 );
    
    std::string m_Name;
	bool m_FreeGirlAble;
	bool m_SlaveAble;
	short m_MinPositions;
	short m_MaxPositions;
	char m_Upkeep;
	char m_GirlPay;
	int m_ActionID;
    std::string m_LikeWork;
    std::string m_HateWork;
	int m_DependantSkill;
};

//mainly a list of functions
class cJobManager
{
public:
	static std::vector<sJobs> job_list;
	void setup_job_list();
	bool generic_job(Girl * girl,int job_num,Brothel *,std::string &,int);
	void WorkFreetime(Girl* girl, Brothel* brothel, int DayNight);
	void WorkTraining(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkBar(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkHall(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkShow(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkWhore(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkCleaning(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkMatron(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkSecurity(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkAdvertising(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	bool WorkTorturer(Girl* girl, Brothel* brothel, int DayNight, std::string& summary);
	void customer_rape(Girl*);
	bool gang_stops_rape(Girl* girl, sGang *gang, int chance, int day_night);
	bool girl_fights_rape(Girl*, int);
	int guard_coverage(std::vector<sGang*> *v = nullptr );
	bool work_related_violence(Girl*, int, bool);
	bool explore_catacombs(Girl *);
	bool beast_capture(Girl * girl,Brothel * brothel,std::string& summary,int);
	bool work_show(Girl * girl,Brothel * brothel,std::string& summary,char job_id,int DayNight);
	void update_film(Brothel *);
	long make_money_films();

	//helpers
    std::string get_job_name(int id){return job_list[id].m_Name;}
	std::vector<Girl*> girls_on_job(Brothel *brothel, u_int job_wanted, int day_or_night);
	Girl **get_training_set(std::vector<Girl*> &v);
	void do_training(Brothel* brothel, int DayNight);
	void do_training_set(Girl **girls, int day_night);
	void do_solo_training(Girl *girl, int day_night);
	
private:
	std::vector<sFilm *> film_list;
};

} // namespace WhoreMasterRenewal

#endif // CJOBS_H_INCLUDED_1524
